// Plays the ring buffer the game's mixer thread fills, resampled to the device's rate
class GameAudioProcessor extends AudioWorkletProcessor
{
	constructor( a_oOptions )
	{
		super();

		const oArgs = a_oOptions.processorOptions;

		this.oSamples = new Float32Array( oArgs.oMemory, oArgs.pRing, oArgs.iFrames * 2 );
		this.oWrite   = new Int32Array( oArgs.oMemory, oArgs.pWrite, 1 );
		this.oRead    = oArgs.pRead ? new Int32Array( oArgs.oMemory, oArgs.pRead, 1 ) : null;
		this.iFrames  = oArgs.iFrames;
		this.fStep    = oArgs.iRate / sampleRate;
		this.iRead    = -1;
		this.fFrac    = 0;
		this.iReport  = 0;
		this.iWorst   = 0;
		this.iGaps    = 0;
		this.iSkips   = 0;
		this.bStarted = false;
		this.iFirst   = -1;
	}

	process( a_aInputs, a_aOutputs )
	{
		const oLeft  = a_aOutputs[ 0 ][ 0 ];
		const oRight = a_aOutputs[ 0 ][ 1 ] || oLeft;

		const iWrite = Atomics.load( this.oWrite, 0 );

		// Gaps count once the mixer has written something
		if ( this.iFirst < 0 )
			this.iFirst = iWrite;
		else if ( iWrite != this.iFirst )
			this.bStarted = true;

		// Stay a little behind the mixer. After a stall the mixer catches up in a burst, then this skips ahead so the delay doesn't stay
		const iAvailable = ( iWrite - this.iRead ) & 0x7FFFFFFF;

		if ( this.iRead < 0 || iAvailable > 4096 )
		{
			if ( this.iRead >= 0 )
				this.iSkips++;

			this.iRead = ( iWrite - 1536 ) & 0x7FFFFFFF;
			this.fFrac = 0;
		}

		this.iWorst = Math.max( this.iWorst, ( iWrite - this.iRead ) & 0x7FFFFFFF );

		const iMask = this.iFrames - 1;

		// The page shows how far behind the mixer this is
		this.iReport += oLeft.length;

		if ( this.iReport >= sampleRate / 2 )
		{
			this.iReport = 0;
			this.port.postMessage( { iLag : ( iWrite - this.iRead ) & 0x7FFFFFFF, iWorst : this.iWorst, iGaps : this.iGaps, iSkips : this.iSkips } );
			this.iWorst = 0;
		}

		let bGap = false;

		for ( let i = 0; i < oLeft.length; i++ )
		{
			// Two frames are needed to blend between them
			if ( ( ( iWrite - this.iRead ) & 0x7FFFFFFF ) < 2 )
			{
				bGap = true;
				oLeft[ i ]  = 0;
				oRight[ i ] = 0;
				continue;
			}


			const iA = ( this.iRead & iMask ) * 2;
			const iB = ( ( this.iRead + 1 ) & iMask ) * 2;
			const f  = this.fFrac;

			oLeft[ i ]  = this.oSamples[ iA ] + ( this.oSamples[ iB ] - this.oSamples[ iA ] ) * f;
			oRight[ i ] = this.oSamples[ iA + 1 ] + ( this.oSamples[ iB + 1 ] - this.oSamples[ iA + 1 ] ) * f;

			this.fFrac += this.fStep;

			while ( this.fFrac >= 1 )
			{
				this.fFrac -= 1;
				this.iRead = ( this.iRead + 1 ) & 0x7FFFFFFF;
			}
		}

		// The mixer keeps a little ahead of this
		if ( this.oRead )
			Atomics.store( this.oRead, 0, this.iRead );

		if ( bGap && this.bStarted )
			this.iGaps++;

		return true;
	}
}

registerProcessor( "game-audio", GameAudioProcessor );
