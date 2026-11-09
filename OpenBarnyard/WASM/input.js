// Gamepads and the on-screen touch controls, the game sees them as one controller (TInputWASMDeviceController)
var oGameInput = {
	// Bits of the controller state, doodad - BUTTON_DPAD_UP
	BUTTON_DPAD_UP : 0,
	BUTTON_DPAD_RIGHT : 1,
	BUTTON_DPAD_DOWN : 2,
	BUTTON_DPAD_LEFT : 3,
	BUTTON_SELECT : 4,
	BUTTON_START : 5,
	BUTTON_STICK_LEFT : 6,
	BUTTON_STICK_RIGHT : 7,
	BUTTON_CONTEXT_1 : 8, // Y
	BUTTON_CONTEXT_2 : 9, // B
	BUTTON_CONTEXT_3 : 10, // A
	BUTTON_CONTEXT_4 : 11, // X
	BUTTON_SHOULDER_LEFT_1 : 12,
	BUTTON_SHOULDER_LEFT_2 : 13,
	BUTTON_SHOULDER_RIGHT_1 : 14,
	BUTTON_SHOULDER_RIGHT_2 : 15,

	uiTouchButtons : 0,
	afTouchSticks : [ 0, 0, 0, 0 ],

	// The glyphs follow the last pad used, the touch buttons are labelled like an Xbox pad
	bSonyPad : false,
	bNoMouse : !window.matchMedia( "(any-pointer: fine)" ).matches,

	Update : function ( a_pState )
	{
		var uiButtons = oGameInput.uiTouchButtons;
		var afSticks  = oGameInput.afTouchSticks.slice();
		var aPads     = navigator.getGamepads ? navigator.getGamepads() : [];

		for ( var i = 0; i < aPads.length; i++ )
		{
			var oPad = aPads[ i ];

			if ( !oPad || !oPad.connected || oPad.mapping != "standard" )
				continue;

			var uiPadButtons = oGameInput.ReadPadButtons( oPad );
			var bPadUsed     = uiPadButtons != 0;
			uiButtons |= uiPadButtons;

			for ( var k = 0; k < 4 && k < oPad.axes.length; k++ )
			{
				if ( Math.abs( oPad.axes[ k ] ) > 0.15 && Math.abs( oPad.axes[ k ] ) > Math.abs( afSticks[ k ] ) )
					afSticks[ k ] = oPad.axes[ k ];

				if ( Math.abs( oPad.axes[ k ] ) > 0.5 )
					bPadUsed = true;
			}

			if ( bPadUsed )
				oGameInput.bSonyPad = oGameInput.IsSonyPad( oPad );
		}

		// Touch buttons, or the PlayStation pad disconnected
		if ( oGameInput.uiTouchButtons || oGameInput.afTouchSticks.some( function ( f ) { return f != 0; } ) )
			oGameInput.bSonyPad = false;
		else if ( oGameInput.bSonyPad && !Array.prototype.some.call( aPads, function ( a_oPad ) { return a_oPad && a_oPad.connected && oGameInput.IsSonyPad( a_oPad ); } ) )
			oGameInput.bSonyPad = false;

		var pWords = a_pState >> 2;

		Module.HEAP32[ pWords ] = uiButtons;

		new Float32Array( Module.HEAP32.buffer, a_pState + 4, 4 ).set( afSticks );
		Module.HEAP32[ pWords + 5 ] = ( oGameInput.bSonyPad ? 1 : 0 ) | ( oGameInput.bNoMouse ? 2 : 0 );
	},

	// Sony's vendor id where the browser shows it, the names Android and others give these pads otherwise
	IsSonyPad : function ( a_oPad )
	{
		return /054c|sony|dualsense|dualshock|playstation|^wireless controller|ps[345] controller/i.test( a_oPad.id );
	},

	ReadPadButtons : function ( a_oPad )
	{
		// The order of the standard gamepad mapping
		var aMap = [
			oGameInput.BUTTON_CONTEXT_3, oGameInput.BUTTON_CONTEXT_2, oGameInput.BUTTON_CONTEXT_4, oGameInput.BUTTON_CONTEXT_1,
			oGameInput.BUTTON_SHOULDER_LEFT_1, oGameInput.BUTTON_SHOULDER_RIGHT_1, oGameInput.BUTTON_SHOULDER_LEFT_2, oGameInput.BUTTON_SHOULDER_RIGHT_2,
			oGameInput.BUTTON_SELECT, oGameInput.BUTTON_START, oGameInput.BUTTON_STICK_LEFT, oGameInput.BUTTON_STICK_RIGHT,
			oGameInput.BUTTON_DPAD_UP, oGameInput.BUTTON_DPAD_DOWN, oGameInput.BUTTON_DPAD_LEFT, oGameInput.BUTTON_DPAD_RIGHT
		];

		var uiButtons = 0;

		for ( var i = 0; i < aMap.length && i < a_oPad.buttons.length; i++ )
		{
			if ( a_oPad.buttons[ i ].pressed )
				uiButtons |= 1 << aMap[ i ];
		}

		return uiButtons;
	},

	IsTouchDevice : function ()
	{
		return location.search.indexOf( "touch" ) >= 0 || window.matchMedia( "(pointer: coarse)" ).matches;
	},

	// The buttons of the overlay, positions are from the bottom right corner
	aTouchButtons : [
		{ szLabel : "A", iBit : 10, fRight : 15, fBottom : 8, fSize : 16 },
		{ szLabel : "B", iBit : 9, fRight : 4, fBottom : 22, fSize : 16 },
		{ szLabel : "X", iBit : 11, fRight : 26, fBottom : 22, fSize : 16 },
		{ szLabel : "Y", iBit : 8, fRight : 15, fBottom : 36, fSize : 16 },
		{ szLabel : "L", iBit : 12, fRight : 46, fBottom : 8, fSize : 13 },
		{ szLabel : "R", iBit : 14, fRight : 46, fBottom : 24, fSize : 13 },
		{ szLabel : "II", iBit : 5, fRight : 4, fBottom : 80, fSize : 11 },
		{ szLabel : "<", iBit : 4, fRight : 18, fBottom : 80, fSize : 11 },
		{ szLabel : "L2", iBit : 13, fRight : 47, fBottom : 40, fSize : 11 },
		{ szLabel : "R2", iBit : 15, fRight : 61, fBottom : 9, fSize : 11 }
	],

	// Saved places and sizes of the touch buttons, by label
	LoadTouchLayout : function ()
	{
		try
		{
			return JSON.parse( localStorage.getItem( "oTouchLayout" ) || "{}" );
		}
		catch ( e )
		{
			return {};
		}
	},

	PlaceTouchButton : function ( a_oElement )
	{
		var oPlace = a_oElement.oPlace;

		a_oElement.style.right      = oPlace.fRight + "vmin";
		a_oElement.style.bottom     = oPlace.fBottom + "vmin";
		a_oElement.style.width      = oPlace.fSize + "vmin";
		a_oElement.style.height     = oPlace.fSize + "vmin";
		a_oElement.style.lineHeight = oPlace.fSize + "vmin";
		a_oElement.style.fontSize   = ( oPlace.fSize * 0.4 ) + "vmin";
	},

	// Drag a button to move it, - and + resize the selected one (all of them when none is)
	EditTouchLayout : function ()
	{
		var oLayer = document.getElementById( "touch" );

		if ( !oLayer || oGameInput.bEditingLayout )
			return;

		oGameInput.bEditingLayout = true;
		oGameInput.uiTouchButtons = 0;
		oGameInput.afTouchSticks.fill( 0 );
		oLayer.classList.add( "editing" );

		var oBar = document.createElement( "div" );
		oBar.id  = "layoutBar";
		oBar.innerHTML = '<span>Drag a button to move it</span><button data-action="smaller">\u2212</button><button data-action="bigger">+</button>' +
			'<button data-action="reset">Reset</button><button data-action="done" class="accent">Done</button>';
		document.body.appendChild( oBar );

		oGameInput.oSelectedButton = null;

		oBar.addEventListener( "click", function ( a_oEvent ) {
			var szAction = a_oEvent.target.dataset && a_oEvent.target.dataset.action;
			var aElements = oGameInput.aTouchElements;

			if ( szAction == "smaller" || szAction == "bigger" )
			{
				for ( var oElement of oGameInput.oSelectedButton ? [ oGameInput.oSelectedButton ] : aElements )
				{
					oElement.oPlace.fSize = Math.max( 8, Math.min( 30, oElement.oPlace.fSize + ( szAction == "bigger" ? 2 : -2 ) ) );
					oGameInput.PlaceTouchButton( oElement );
				}
			}
			else if ( szAction == "reset" )
			{
				for ( var oElement of aElements )
				{
					oElement.oPlace = Object.assign( {}, oElement.oDefault );
					oGameInput.PlaceTouchButton( oElement );
				}
			}
			else if ( szAction == "done" )
			{
				var oLayout = {};

				for ( var oElement of aElements )
					oLayout[ oElement.oDefault.szLabel ] = { fRight : oElement.oPlace.fRight, fBottom : oElement.oPlace.fBottom, fSize : oElement.oPlace.fSize };

				try
				{
					localStorage.setItem( "oTouchLayout", JSON.stringify( oLayout ) );
				}
				catch ( e )
				{
				}

				if ( oGameInput.oSelectedButton )
					oGameInput.oSelectedButton.classList.remove( "selected" );

				oLayer.classList.remove( "editing" );
				oBar.remove();
				oGameInput.bEditingLayout = false;
			}
		} );
	},

	CreateTouchControls : function ()
	{
		var oLayer = document.createElement( "div" );
		oLayer.id  = "touch";

		document.body.appendChild( oLayer );

		var oStickBase = document.createElement( "div" );
		oStickBase.className = "stick";
		oLayer.appendChild( oStickBase );

		var oStickKnob = document.createElement( "div" );
		oStickKnob.className = "knob";
		oStickBase.appendChild( oStickKnob );

		var oLayout = oGameInput.LoadTouchLayout();
		oGameInput.aTouchElements = [];

		for ( var oButton of oGameInput.aTouchButtons )
		{
			var oElement = document.createElement( "div" );

			oElement.className    = "button";
			oElement.textContent  = oButton.szLabel;
			oElement.dataset.iBit = oButton.iBit;
			oElement.oDefault     = oButton;
			oElement.oPlace       = Object.assign( {}, oButton, oLayout[ oButton.szLabel ] || {} );

			oGameInput.PlaceTouchButton( oElement );
			oGameInput.aTouchElements.push( oElement );
			oLayer.appendChild( oElement );
		}

		// Layout editing: a touch on a button selects and drags it
		var oDrag = null;

		function VMin()
		{
			return Math.min( window.innerWidth, window.innerHeight ) / 100;
		}

		// A touch that starts on a button stays that button, the left half moves, the right half looks around
		var oTouches = {};

		function Radius()
		{
			return Math.min( window.innerWidth, window.innerHeight ) * 0.15;
		}

		function UpdateStick( a_oTouch, a_oState )
		{
			var fX = ( a_oTouch.clientX - a_oState.fStartX ) / Radius();
			var fY = ( a_oTouch.clientY - a_oState.fStartY ) / Radius();
			var fLength = Math.sqrt( fX * fX + fY * fY );

			if ( fLength > 1 )
			{
				fX /= fLength;
				fY /= fLength;
			}

			oGameInput.afTouchSticks[ a_oState.iStick * 2 + 0 ] = fX;
			oGameInput.afTouchSticks[ a_oState.iStick * 2 + 1 ] = fY;

			if ( a_oState.iStick == 0 )
				oStickKnob.style.transform = "translate(" + ( fX * 100 ) + "%," + ( fY * 100 ) + "%)";
		}

		function OnStart( a_oEvent )
		{
			a_oEvent.preventDefault();

			if ( oGameInput.bEditingLayout )
			{
				var oTouch  = a_oEvent.changedTouches[ 0 ];
				var oTarget = oTouch.target;

				if ( oGameInput.oSelectedButton )
					oGameInput.oSelectedButton.classList.remove( "selected" );

				oGameInput.oSelectedButton = oTarget.oPlace ? oTarget : null;

				if ( oTarget.oPlace )
				{
					oTarget.classList.add( "selected" );
					oDrag = { oElement : oTarget, iId : oTouch.identifier, fX : oTouch.clientX, fY : oTouch.clientY, fRight : oTarget.oPlace.fRight, fBottom : oTarget.oPlace.fBottom };
				}

				return;
			}

			for ( var oTouch of a_oEvent.changedTouches )
			{
				var oTarget = oTouch.target;

				if ( oTarget.dataset && oTarget.dataset.iBit !== undefined )
				{
					oTouches[ oTouch.identifier ] = { iBit : +oTarget.dataset.iBit, oElement : oTarget };
					oGameInput.uiTouchButtons |= 1 << +oTarget.dataset.iBit;
					oTarget.classList.add( "down" );
					continue;
				}

				var iStick = oTouch.clientX < window.innerWidth / 2 ? 0 : 1;

				oTouches[ oTouch.identifier ] = { iStick : iStick, fStartX : oTouch.clientX, fStartY : oTouch.clientY };

				if ( iStick == 0 )
				{
					oStickBase.style.left    = oTouch.clientX + "px";
					oStickBase.style.top     = oTouch.clientY + "px";
					oStickBase.style.display = "block";
				}
			}
		}

		function OnMove( a_oEvent )
		{
			a_oEvent.preventDefault();

			if ( oGameInput.bEditingLayout )
			{
				for ( var oTouch of a_oEvent.changedTouches )
				{
					if ( !oDrag || oTouch.identifier != oDrag.iId )
						continue;

					var oPlace  = oDrag.oElement.oPlace;
					var fVMin   = VMin();
					var fMaxX   = window.innerWidth / fVMin - oPlace.fSize;
					var fMaxY   = window.innerHeight / fVMin - oPlace.fSize;
					oPlace.fRight  = Math.max( 0, Math.min( fMaxX, oDrag.fRight - ( oTouch.clientX - oDrag.fX ) / fVMin ) );
					oPlace.fBottom = Math.max( 0, Math.min( fMaxY, oDrag.fBottom - ( oTouch.clientY - oDrag.fY ) / fVMin ) );
					oGameInput.PlaceTouchButton( oDrag.oElement );
				}

				return;
			}

			for ( var oTouch of a_oEvent.changedTouches )
			{
				var oState = oTouches[ oTouch.identifier ];

				if ( oState && oState.iStick !== undefined )
					UpdateStick( oTouch, oState );
			}
		}

		function OnEnd( a_oEvent )
		{
			a_oEvent.preventDefault();

			if ( oGameInput.bEditingLayout )
			{
				oDrag = null;
				return;
			}

			for ( var oTouch of a_oEvent.changedTouches )
			{
				var oState = oTouches[ oTouch.identifier ];

				if ( !oState )
					continue;

				if ( oState.iStick !== undefined )
				{
					oGameInput.afTouchSticks[ oState.iStick * 2 + 0 ] = 0;
					oGameInput.afTouchSticks[ oState.iStick * 2 + 1 ] = 0;

					if ( oState.iStick == 0 )
					{
						oStickBase.style.display = "none";
						oStickKnob.style.transform = "";
					}
				}
				else
				{
					oGameInput.uiTouchButtons &= ~( 1 << oState.iBit );
					oState.oElement.classList.remove( "down" );
				}

				delete oTouches[ oTouch.identifier ];
			}
		}

		oLayer.addEventListener( "touchstart", OnStart, { passive : false } );
		oLayer.addEventListener( "touchmove", OnMove, { passive : false } );
		oLayer.addEventListener( "touchend", OnEnd, { passive : false } );
		oLayer.addEventListener( "touchcancel", OnEnd, { passive : false } );

		// iPhones only go fullscreen as a Home Screen app
		if ( !document.documentElement.requestFullscreen && !document.documentElement.webkitRequestFullscreen && !navigator.standalone && !/[?&]app(&|$)/.test( location.search ) )
			oGameInput.ShowHomeScreenHint();

		// Android can go fullscreen and hold landscape from a gesture, iPhones ignore this
		oLayer.addEventListener( "touchend", function () {
			if ( document.documentElement.requestFullscreen && !document.fullscreenElement )
			{
				document.documentElement.requestFullscreen().then( function () {
					if ( screen.orientation && screen.orientation.lock )
						return screen.orientation.lock( "landscape" );
				} ).catch( function () {} );
			}
		}, { once : true } );
	},

	ShowHomeScreenHint : function ()
	{
		try
		{
			if ( localStorage.getItem( "bHomeHintSeen" ) )
				return;
		}
		catch ( e )
		{
		}

		var oHint = document.createElement( "div" );
		oHint.id  = "homeHint";
		oHint.innerHTML = "<span>For fullscreen, tap Share and then Add to Home Screen</span><button>OK</button>";
		document.body.appendChild( oHint );

		oHint.querySelector( "button" ).addEventListener( "click", function () {
			oHint.remove();

			try
			{
				localStorage.setItem( "bHomeHintSeen", "1" );
			}
			catch ( e )
			{
			}
		} );
	},

	// The mouse moves the game's own cursor and camera, so the real one is locked
	EnablePointerLock : function ()
	{
		Module.oCanvas.addEventListener( "pointerdown", function ( a_oEvent ) {
			if ( a_oEvent.pointerType == "mouse" && document.pointerLockElement != Module.oCanvas && Module.oCanvas.requestPointerLock )
			{
				var oResult = Module.oCanvas.requestPointerLock();

				if ( oResult && oResult.catch )
					oResult.catch( function () {} );
			}
		} );
	},

	Start : function ()
	{
		if ( oGameInput.IsTouchDevice() )
			oGameInput.CreateTouchControls();
		else
			oGameInput.EnablePointerLock();
	}
};
