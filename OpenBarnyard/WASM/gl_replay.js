// Runs the WebGL calls the game thread writes to its ring buffer (Toshi/Source/Platform/WASM/TCGLStream_WASM.cpp)
var oGLReplay = {
	bDrawnInFrame : false,
	gl : null,
	aObjects : [ null ],
	aLocations : [],
	oSources : {},
	oMemory : null,
	H32 : null,
	HF32 : null,
	HU8 : null,

	IsSlowGPU : function ()
	{
		if ( oGLReplay.bSlowGPU === undefined )
		{
			var gl    = document.createElement( "canvas" ).getContext( "webgl2" );
			var oInfo = gl && gl.getExtension( "WEBGL_debug_renderer_info" );
			var szGPU = oInfo ? gl.getParameter( oInfo.UNMASKED_RENDERER_WEBGL ) : "";

			oGLReplay.bSlowGPU = /PowerVR/i.test( szGPU );
		}

		return oGLReplay.bSlowGPU;
	},

	Create : function ( a_iWidth, a_iHeight )
	{
		Module.oCanvas.width  = a_iWidth;
		Module.oCanvas.height = a_iHeight;

		oGLReplay.gl = Module.oCanvas.getContext( "webgl2", {
			alpha : false,
			depth : true,
			stencil : true,
			antialias : false,
			premultipliedAlpha : false,
			preserveDrawingBuffer : false,
			powerPreference : "high-performance",
			desynchronized : oPageSettings.IsLowLatency()
		} );

		if ( !oGLReplay.gl )
			return 0;

		// Everything on the GPU is gone with the context and the game has no way to upload it again.
		// The saves are safe, so start over
		Module.oCanvas.addEventListener( "webglcontextlost", function ( a_oEvent ) {
			a_oEvent.preventDefault();
			oGLReplay.bLost = true;
			document.getElementById( "status" ).textContent = "Restarting...";
		} );

		Module.oCanvas.addEventListener( "webglcontextrestored", function () {
			location.reload();
		} );

		return 1;
	},

	Views : function ()
	{
		var oBuffer = Module.wasmMemory.buffer;

		if ( oBuffer !== oGLReplay.oMemory )
		{
			oGLReplay.oMemory = oBuffer;
			oGLReplay.H32     = new Int32Array( oBuffer );
			oGLReplay.HF32    = new Float32Array( oBuffer );
			oGLReplay.HU8     = new Uint8Array( oBuffer );
		}
	},

	AddShaderSource : function ( a_iId, a_szText, a_bLast )
	{
		oGLReplay.oSources[ a_iId ] = ( oGLReplay.oSources[ a_iId ] || "" ) + a_szText;

		if ( a_bLast )
		{
			oGLReplay.gl.shaderSource( oGLReplay.aObjects[ a_iId ], oGLReplay.oSources[ a_iId ] );
			delete oGLReplay.oSources[ a_iId ];
		}
	},

	GetShaderParameter : function ( a_iId, a_ePName )
	{
		return Number( oGLReplay.gl.getShaderParameter( oGLReplay.aObjects[ a_iId ], a_ePName ) );
	},

	GetProgramParameter : function ( a_iId, a_ePName )
	{
		return Number( oGLReplay.gl.getProgramParameter( oGLReplay.aObjects[ a_iId ], a_ePName ) );
	},

	GetShaderLog : function ( a_iId, a_pOut, a_iSize )
	{
		Module.stringToUTF8( oGLReplay.gl.getShaderInfoLog( oGLReplay.aObjects[ a_iId ] ) || "", a_pOut, a_iSize );
	},

	GetProgramLog : function ( a_iId, a_pOut, a_iSize )
	{
		Module.stringToUTF8( oGLReplay.gl.getProgramInfoLog( oGLReplay.aObjects[ a_iId ] ) || "", a_pOut, a_iSize );
	},

	GetUniformBlockIndex : function ( a_iId, a_szName )
	{
		return oGLReplay.gl.getUniformBlockIndex( oGLReplay.aObjects[ a_iId ], a_szName );
	},

	GetUniformBlockParameter : function ( a_iId, a_iBlock, a_ePName )
	{
		return oGLReplay.gl.getActiveUniformBlockParameter( oGLReplay.aObjects[ a_iId ], a_iBlock, a_ePName );
	},

	GetUniformLocation : function ( a_iId, a_szName )
	{
		var oLocation = oGLReplay.gl.getUniformLocation( oGLReplay.aObjects[ a_iId ], a_szName );

		if ( !oLocation )
			return -1;

		oGLReplay.aLocations.push( oLocation );
		return oGLReplay.aLocations.length - 1;
	},

	ReadPixels : function ( a_iX, a_iY, a_iWidth, a_iHeight, a_eFormat, a_eType, a_pOut )
	{
		oGLReplay.Views();
		oGLReplay.gl.readPixels( a_iX, a_iY, a_iWidth, a_iHeight, a_eFormat, a_eType, oGLReplay.HU8, a_pOut );
	},

	CreateObject : function ( a_eType, a_iId )
	{
		var gl = oGLReplay.gl;
		var oObject;

		switch ( a_eType )
		{
			case 0: oObject = gl.createBuffer(); break;
			case 1: oObject = gl.createTexture(); break;
			case 2: oObject = gl.createSampler(); break;
			case 3: oObject = gl.createVertexArray(); break;
			case 4: oObject = gl.createProgram(); break;
			case 5: oObject = gl.createShader( gl.VERTEX_SHADER ); break;
			case 6: oObject = gl.createShader( gl.FRAGMENT_SHADER ); break;
		}

		oGLReplay.aObjects[ a_iId ] = oObject;
	},

	DeleteObject : function ( a_eType, a_iId )
	{
		var gl      = oGLReplay.gl;
		var oObject = oGLReplay.aObjects[ a_iId ];

		switch ( a_eType )
		{
			case 0: gl.deleteBuffer( oObject ); break;
			case 1: gl.deleteTexture( oObject ); break;
			case 2: gl.deleteSampler( oObject ); break;
			case 3: gl.deleteVertexArray( oObject ); break;
			case 4: gl.deleteProgram( oObject ); break;
			default: gl.deleteShader( oObject ); break;
		}

		oGLReplay.aObjects[ a_iId ] = null;
	},

	Replay : function ()
	{
		if ( !oGLReplay.pState )
		{
			oGLReplay.pState = Module._WASM_GetGLStream() >> 2;
		}

		oGLReplay.Views();

		var gl     = oGLReplay.gl;
		var H      = oGLReplay.H32;
		var F      = oGLReplay.HF32;
		var U      = oGLReplay.HU8;
		var O      = oGLReplay.aObjects;
		var L      = oGLReplay.aLocations;
		var pState = oGLReplay.pState;
		var pRing  = H[ pState + 3 ] >> 2;
		var uiMask = H[ pState + 4 ];

		var uiRead      = Atomics.load( H, pState + 0 );
		var uiPublished = Atomics.load( H, pState + 1 );
		var iPresented  = 0;
		var fWaitUntil  = 0;

		for ( ;; )
		{
			if ( uiRead == uiPublished )
			{
				// The game sends a frame in parts when it uploads a lot while drawing (new terrain). Ending here would
				// show the half drawn frame, so the rest is waited for, for a while
				if ( !oGLReplay.bDrawnInFrame )
					break;

				Atomics.store( H, pState + 0, uiRead );
				Atomics.notify( H, pState + 0 );

				if ( !fWaitUntil )
					fWaitUntil = performance.now() + 100;

				while ( uiRead == ( uiPublished = Atomics.load( H, pState + 1 ) ) && performance.now() < fWaitUntil )
				{
				}

				if ( uiRead == uiPublished )
					break;
			}

			var i = pRing + ( uiRead & uiMask );
			var n = 1;

			switch ( H[ i ] )
			{
				case 0: n = uiMask + 1 - ( uiRead & uiMask ); break;
				case 1:
					iPresented++;
					oGLReplay.bDrawnInFrame = false;
					break;
				case 2:
					Module.oCanvas.width  = H[ i + 1 ];
					Module.oCanvas.height = H[ i + 2 ];
					n = 3;
					break;
				case 3: oGLReplay.CreateObject( H[ i + 1 ], H[ i + 2 ] ); n = 3; break;
				case 4: oGLReplay.DeleteObject( H[ i + 1 ], H[ i + 2 ] ); n = 3; break;
				case 5: gl.activeTexture( H[ i + 1 ] ); n = 2; break;
				case 6: gl.attachShader( O[ H[ i + 1 ] ], O[ H[ i + 2 ] ] ); n = 3; break;
				case 7: gl.bindBuffer( H[ i + 1 ], O[ H[ i + 2 ] ] ); n = 3; break;
				case 8: gl.bindBufferBase( H[ i + 1 ], H[ i + 2 ], O[ H[ i + 3 ] ] ); n = 4; break;
				case 9: gl.bindFramebuffer( H[ i + 1 ], null ); n = 3; break;
				case 10: gl.bindSampler( H[ i + 1 ], O[ H[ i + 2 ] ] ); n = 3; break;
				case 11: gl.bindTexture( H[ i + 1 ], O[ H[ i + 2 ] ] ); n = 3; break;
				case 12: gl.bindVertexArray( O[ H[ i + 1 ] ] ); n = 2; break;
				case 13: gl.blendColor( F[ i + 1 ], F[ i + 2 ], F[ i + 3 ], F[ i + 4 ] ); n = 5; break;
				case 14: gl.blendEquationSeparate( H[ i + 1 ], H[ i + 2 ] ); n = 3; break;
				case 15: gl.blendFuncSeparate( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ] ); n = 5; break;
				case 16: // bufferData: target, size, usage, pointer, words
				{
					var pData  = H[ i + 4 ];
					var iWords = H[ i + 5 ];

					if ( pData == 0 && iWords == 0 )
						gl.bufferData( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ] );
					else
						gl.bufferData( H[ i + 1 ], U, H[ i + 3 ], pData || ( i + 6 ) << 2, H[ i + 2 ] );

					n = 6 + iWords;
					break;
				}
				case 17: // bufferSubData: target, offset, size, pointer, words
				{
					var pData  = H[ i + 4 ];
					var iWords = H[ i + 5 ];

					gl.bufferSubData( H[ i + 1 ], H[ i + 2 ], U, pData || ( i + 6 ) << 2, H[ i + 3 ] );

					n = 6 + iWords;
					break;
				}
				case 18: gl.clear( H[ i + 1 ] ); n = 2; break;
				case 19: gl.clearColor( F[ i + 1 ], F[ i + 2 ], F[ i + 3 ], F[ i + 4 ] ); n = 5; break;
				case 20: gl.clearDepth( F[ i + 1 ] ); n = 2; break;
				case 21: gl.clearStencil( H[ i + 1 ] ); n = 2; break;
				case 22: gl.colorMask( !!H[ i + 1 ], !!H[ i + 2 ], !!H[ i + 3 ], !!H[ i + 4 ] ); n = 5; break;
				case 23: gl.compileShader( O[ H[ i + 1 ] ] ); n = 2; break;
				case 24: // compressedTexImage2D: target, level, format, width, height, size, pointer, words
				{
					var pData  = H[ i + 7 ];
					var iWords = H[ i + 8 ];

					gl.compressedTexImage2D( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ], H[ i + 5 ], 0, U, pData || ( i + 9 ) << 2, H[ i + 6 ] );

					n = 9 + iWords;
					break;
				}
				case 25: gl.cullFace( H[ i + 1 ] ); n = 2; break;
				case 26: gl.depthFunc( H[ i + 1 ] ); n = 2; break;
				case 27: gl.depthMask( !!H[ i + 1 ] ); n = 2; break;
				case 28: gl.depthRange( F[ i + 1 ], F[ i + 2 ] ); n = 3; break;
				case 29: gl.disable( H[ i + 1 ] ); n = 2; break;
				case 30: gl.disableVertexAttribArray( H[ i + 1 ] ); n = 2; break;
				case 31: gl.drawArrays( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ] ); oGLReplay.bDrawnInFrame = true; n = 4; break;
				case 32: gl.drawElements( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ] ); oGLReplay.bDrawnInFrame = true; n = 5; break;
				case 33: gl.enable( H[ i + 1 ] ); n = 2; break;
				case 34: gl.enableVertexAttribArray( H[ i + 1 ] ); n = 2; break;
				case 35: gl.flush(); break;
				case 36: gl.frontFace( H[ i + 1 ] ); n = 2; break;
				case 37: gl.generateMipmap( H[ i + 1 ] ); n = 2; break;
				case 38: gl.linkProgram( O[ H[ i + 1 ] ] ); n = 2; break;
				case 39: gl.pixelStorei( H[ i + 1 ], H[ i + 2 ] ); n = 3; break;
				case 40: gl.polygonOffset( F[ i + 1 ], F[ i + 2 ] ); n = 3; break;
				case 41: gl.samplerParameteri( O[ H[ i + 1 ] ], H[ i + 2 ], H[ i + 3 ] ); n = 4; break;
				case 42: gl.scissor( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ] ); n = 5; break;
				case 43: gl.stencilFunc( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ] ); n = 4; break;
				case 44: gl.stencilMask( H[ i + 1 ] ); n = 2; break;
				case 45: gl.stencilOp( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ] ); n = 4; break;
				case 46: // texImage2D: target, level, internal format, width, height, format, type, pointer, words
				{
					var pData  = H[ i + 8 ];
					var iWords = H[ i + 9 ];

					if ( pData == 0 && iWords == 0 )
						gl.texImage2D( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ], H[ i + 5 ], 0, H[ i + 6 ], H[ i + 7 ], null );
					else
						gl.texImage2D( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ], H[ i + 5 ], 0, H[ i + 6 ], H[ i + 7 ], U, pData || ( i + 10 ) << 2 );

					n = 10 + iWords;
					break;
				}
				case 47: gl.texParameteri( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ] ); n = 4; break;
				case 48: // texSubImage2D: target, level, x, y, width, height, format, type, pointer, words
				{
					var pData  = H[ i + 9 ];
					var iWords = H[ i + 10 ];

					gl.texSubImage2D( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ], H[ i + 5 ], H[ i + 6 ], H[ i + 7 ], H[ i + 8 ], U, pData || ( i + 11 ) << 2 );

					n = 11 + iWords;
					break;
				}
				case 49: gl.uniform1i( L[ H[ i + 1 ] ], H[ i + 2 ] ); n = 3; break;
				case 50: gl.uniformBlockBinding( O[ H[ i + 1 ] ], H[ i + 2 ], H[ i + 3 ] ); n = 4; break;
				case 51: gl.useProgram( O[ H[ i + 1 ] ] ); n = 2; break;
				case 52: gl.vertexAttribPointer( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], !!H[ i + 4 ], H[ i + 5 ], H[ i + 6 ] ); n = 7; break;
				case 53: gl.viewport( H[ i + 1 ], H[ i + 2 ], H[ i + 3 ], H[ i + 4 ] ); n = 5; break;
				default:
					console.error( "gl_replay: bad command " + H[ i ] + " at " + uiRead );
					return;
			}

			uiRead = ( uiRead + n ) | 0;
		}

		Atomics.store( H, pState + 0, uiRead );
		Atomics.notify( H, pState + 0 );

		if ( iPresented > 0 )
		{
			Atomics.add( H, pState + 2, iPresented );
			Atomics.notify( H, pState + 2 );
			Module.OnFramesReplayed( iPresented );
		}
	}
};
