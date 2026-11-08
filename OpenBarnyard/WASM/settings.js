// The page's own settings (gear button in the top left corner), kept in localStorage.
// The game picks its resolution and canvas once, so changing those restarts it
var oPageSettings = {
	oValues : { szResolution : "auto", szLowLatency : "auto", bShowFps : false, bSlowDevice : false },

	Load : function ()
	{
		var oStored = {};

		try
		{
			oStored = JSON.parse( localStorage.getItem( "oPageSettings" ) || "{}" );

			for ( var szKey in oPageSettings.oValues )
			{
				if ( oStored[ szKey ] !== undefined )
					oPageSettings.oValues[ szKey ] = oStored[ szKey ];
			}
		}
		catch ( e )
		{
		}

		// ?fps is only the default, the app always starts with it
		if ( location.search.indexOf( "fps" ) >= 0 && oStored.bShowFps === undefined )
			oPageSettings.oValues.bShowFps = true;
	},

	Save : function ()
	{
		try
		{
			localStorage.setItem( "oPageSettings", JSON.stringify( oPageSettings.oValues ) );
		}
		catch ( e )
		{
		}
	},

	// A known slow GPU, or one that ran slowly before
	IsSlowDevice : function ()
	{
		return oGLReplay.IsSlowGPU() || oPageSettings.oValues.bSlowDevice;
	},

	// The most lines the game draws, 0 for the whole screen
	GetMaxHeight : function ()
	{
		var szResolution = oPageSettings.oValues.szResolution;

		if ( szResolution == "full" )
			return 0;

		if ( szResolution != "auto" )
			return +szResolution;

		if ( !oGameInput.IsTouchDevice() )
			return 0;

		return oPageSettings.IsSlowDevice() ? 540 : 720;
	},

	IsLowLatency : function ()
	{
		var szLowLatency = oPageSettings.oValues.szLowLatency;

		if ( szLowLatency != "auto" )
			return szLowLatency == "on";

		return oPageSettings.IsSlowDevice();
	},

	// In auto mode a device that stays under 22 FPS for 30 seconds is slow from the next start on
	WatchFrameRate : function ()
	{
		if ( oPageSettings.oValues.szResolution != "auto" || oPageSettings.IsSlowDevice() || !oGameInput.IsTouchDevice() )
			return;

		var iLastFrames = 0, iSlowWindows = 0, iWindows = 0;

		var iTimer = setInterval( function () {
			var iFrames = Module.oStats.iFrames - iLastFrames;
			iLastFrames = Module.oStats.iFrames;

			// The first minute is loading
			if ( document.hidden || ++iWindows <= 6 )
				return;

			iSlowWindows = iFrames < 220 ? iSlowWindows + 1 : 0;

			if ( iSlowWindows >= 3 )
			{
				oPageSettings.oValues.bSlowDevice = true;
				oPageSettings.Save();
				clearInterval( iTimer );
				PageLog( "slow device, auto settings change on the next start" );
			}
		}, 10000 );
	},

	// The cheat codes are typed while Tab is held, phones have no keyboard for that
	EnterCheatCode : function ( a_szCode )
	{
		var aKeys = [ "Tab" ];

		for ( var szLetter of a_szCode.toUpperCase() )
		{
			if ( szLetter >= "A" && szLetter <= "Z" )
				aKeys.push( "Key" + szLetter );
		}

		if ( aKeys.length == 1 )
			return;

		function Send( a_szType, a_szCode )
		{
			window.dispatchEvent( new KeyboardEvent( a_szType, { code : a_szCode, key : a_szCode.replace( "Key", "" ).toLowerCase(), bubbles : true } ) );
		}

		// A few frames per key so the game sees each one, even when slow
		var iStep = 0;

		var iTimer = setInterval( function () {
			var iKey = iStep >> 1;

			if ( iKey == aKeys.length )
			{
				Send( "keyup", "Tab" );
				clearInterval( iTimer );
				return;
			}

			if ( iKey == 0 )
			{
				if ( !( iStep & 1 ) )
					Send( "keydown", "Tab" );
			}
			else
			{
				Send( iStep & 1 ? "keyup" : "keydown", aKeys[ iKey ] );
			}

			iStep++;
		}, 120 );
	},

	ShowFps : function ( a_bShow )
	{
		var oFps = document.getElementById( "fps" );

		if ( !a_bShow )
		{
			if ( oFps )
			{
				clearInterval( oPageSettings.iFpsTimer );
				oFps.remove();
			}

			return;
		}

		if ( oFps )
			return;

		oFps    = document.createElement( "div" );
		oFps.id = "fps";
		document.body.appendChild( oFps );

		var iFpsFrames = Module.oStats.iFrames, fFpsTime = performance.now();

		oPageSettings.iFpsTimer = setInterval( function () {
			var fNow = performance.now();

			oFps.textContent = Math.round( ( Module.oStats.iFrames - iFpsFrames ) * 1000 / ( fNow - fFpsTime ) ) + " FPS";

			iFpsFrames = Module.oStats.iFrames;
			fFpsTime   = fNow;
		}, 500 );
	},

	CreatePanel : function ()
	{
		var oButton = document.createElement( "div" );
		oButton.id  = "settingsButton";
		oButton.innerHTML = '<svg viewBox="0 0 24 24" width="100%" height="100%"><path fill="currentColor" d="M19.4 13a7.5 7.5 0 0 0 0-2l2.1-1.6-2-3.5-2.5 1a7.4 7.4 0 0 0-1.7-1L15 3h-4l-.3 2.9a7.4 7.4 0 0 0-1.7 1l-2.5-1-2 3.5L6.6 11a7.5 7.5 0 0 0 0 2l-2.1 1.6 2 3.5 2.5-1a7.4 7.4 0 0 0 1.7 1L11 21h4l.3-2.9a7.4 7.4 0 0 0 1.7-1l2.5 1 2-3.5zM13 15.5a3.5 3.5 0 1 1 0-7 3.5 3.5 0 0 1 0 7z" transform="translate(-1 0)"/></svg>';
		document.body.appendChild( oButton );

		var oPanel = document.createElement( "div" );
		oPanel.id  = "settings";
		oPanel.innerHTML =
			'<div class="head"><div class="title">Settings</div>' +
			'<div class="about">Based on <a href="https://github.com/InfiniteC0re/OpenBarnyard" target="_blank" rel="noopener">OpenBarnyard decompilation</a></div></div>' +
			'<div class="section">Display</div>' +
			'<label>Resolution <select data-key="szResolution">' +
			'<option value="auto">Auto</option><option value="540">540 lines</option><option value="720">720 lines</option>' +
			'<option value="1080">1080 lines</option><option value="full">Screen</option></select></label>' +
			'<label>Low latency <select data-key="szLowLatency">' +
			'<option value="auto">Auto</option><option value="on">On</option><option value="off">Off</option></select></label>' +
			'<label>Show FPS <input type="checkbox" data-key="bShowFps"></label>' +
			'<div class="hint" id="settingsAuto"></div>' +
			'<div class="section">Cheats</div>' +
			'<div class="row"><input type="text" id="settingsCheat" placeholder="Cheat code" autocomplete="off" autocapitalize="characters" spellcheck="false"><button id="settingsSendCheat">Enter</button></div>' +
			'<div class="hint" id="settingsRestartHint">Restarts the game. Saves are kept, progress since the last save is lost.</div>' +
			'<div class="buttons"><button id="settingsRestart" class="accent">Restart</button><button id="settingsClose">Close</button></div>';
		document.body.appendChild( oPanel );

		var oStarted = {
			szResolution : oPageSettings.oValues.szResolution,
			szLowLatency : oPageSettings.oValues.szLowLatency
		};

		function UpdatePanel()
		{
			var bRestart = oStarted.szResolution != oPageSettings.oValues.szResolution || oStarted.szLowLatency != oPageSettings.oValues.szLowLatency;

			document.getElementById( "settingsRestart" ).style.display     = bRestart ? "" : "none";
			document.getElementById( "settingsRestartHint" ).style.display = bRestart ? "" : "none";
			document.getElementById( "settingsAuto" ).textContent = "Now " + Module.oCanvas.width + "x" + Module.oCanvas.height + ( oPageSettings.IsSlowDevice() ? ", auto treats this device as slow" : "" );
		}

		for ( var oControl of oPanel.querySelectorAll( "[data-key]" ) )
		{
			var szKey = oControl.dataset.key;

			if ( oControl.type == "checkbox" )
				oControl.checked = oPageSettings.oValues[ szKey ];
			else
				oControl.value = oPageSettings.oValues[ szKey ];

			oControl.addEventListener( "change", function ( a_oEvent ) {
				var oTarget = a_oEvent.target;
				oPageSettings.oValues[ oTarget.dataset.key ] = oTarget.type == "checkbox" ? oTarget.checked : oTarget.value;

				// Picking auto again measures the device again
				if ( oTarget.dataset.key == "szResolution" && oTarget.value == "auto" )
					oPageSettings.oValues.bSlowDevice = false;

				oPageSettings.Save();
				oPageSettings.ShowFps( oPageSettings.oValues.bShowFps );
				UpdatePanel();
			} );
		}

		oButton.addEventListener( "click", function () {
			if ( document.pointerLockElement )
				document.exitPointerLock();

			UpdatePanel();
			oPanel.style.display = oPanel.style.display == "block" ? "none" : "block";
		} );

		document.getElementById( "settingsClose" ).addEventListener( "click", function () {
			oPanel.style.display = "none";
		} );

		document.getElementById( "settingsRestart" ).addEventListener( "click", function () {
			location.reload();
		} );

		document.getElementById( "settingsSendCheat" ).addEventListener( "click", function () {
			var oInput = document.getElementById( "settingsCheat" );

			oInput.blur();
			oPanel.style.display = "none";
			oPageSettings.EnterCheatCode( oInput.value );
			oInput.value = "";
		} );

		// The game listens on the window in the capture phase, typing in the panel isn't game input
		for ( var szEvent of [ "keydown", "keyup", "keypress" ] )
		{
			window.addEventListener( szEvent, function ( a_oEvent ) {
				if ( a_oEvent.isTrusted && oPanel.contains( a_oEvent.target ) )
					a_oEvent.stopImmediatePropagation();
			}, true );
		}
	}
};

oPageSettings.Load();
oPageSettings.CreatePanel();
