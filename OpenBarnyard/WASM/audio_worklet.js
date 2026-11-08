// Plays the ring buffer the game's mixer thread fills
class GameAudioProcessor extends AudioWorkletProcessor
{
	constructor( a_oOptions )
	{
		super();

		const oArgs = a_oOptions.processorOptions;

		this.oSamples = new Float32Array( oArgs.oMemory, oArgs.pRing, oArgs.iFrames * 2 );
		this.oWrite   = new Int32Array( oArgs.oMemory, oArgs.pWrite, 1 );
		this.iFrames  = oArgs.iFrames;
		this.iRead    = -1;
	}

	process( a_aInputs, a_aOutputs )
	{
		const oLeft  = a_aOutputs[ 0 ][ 0 ];
		const oRight = a_aOutputs[ 0 ][ 1 ] || oLeft;

		const iWrite = Atomics.load( this.oWrite, 0 );

		// Stay a little behind the mixer, and catch up if it ran ahead
		const iAvailable = ( iWrite - this.iRead ) & 0x7FFFFFFF;

		if ( this.iRead < 0 || iAvailable > this.iFrames - 1024 )
			this.iRead = ( iWrite - 2048 ) & 0x7FFFFFFF;

		for ( let i = 0; i < oLeft.length; i++ )
		{
			if ( ( ( iWrite - this.iRead ) & 0x7FFFFFFF ) == 0 )
			{
				oLeft[ i ]  = 0;
				oRight[ i ] = 0;
				continue;
			}

			const iAt = this.iRead & ( this.iFrames - 1 );

			oLeft[ i ]  = this.oSamples[ iAt * 2 + 0 ];
			oRight[ i ] = this.oSamples[ iAt * 2 + 1 ];

			this.iRead = ( this.iRead + 1 ) & 0x7FFFFFFF;
		}

		return true;
	}
}

registerProcessor( "game-audio", GameAudioProcessor );
