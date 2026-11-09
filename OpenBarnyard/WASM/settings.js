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

		// A press or release every two game frames (the game may read input a frame after the page shows one),
		// so the game sees each one however fast it runs
		var iStep = 0, iLastFrames = -2, fLastStep = 0;

		var iTimer = setInterval( function () {
			var iFrames = Module.oStats ? Module.oStats.iFrames : 0;
			var fNow    = performance.now();

			// Two frames passed, or the game isn't drawing (hidden) and it's been a while
			if ( iFrames - iLastFrames < 2 && fNow - fLastStep < 1000 )
				return;

			iLastFrames = iFrames;
			fLastStep   = fNow;

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
		}, 4 );
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
			'<div class="buttons" id="settingsTouchRow"><button id="settingsTouchLayout">Edit touch buttons</button></div>' +
			'<div class="section">Cheats</div>' +
			'<div class="row"><input type="text" id="settingsCheat" placeholder="Cheat code" autocomplete="off" autocapitalize="characters" spellcheck="false"><button id="settingsSendCheat">Enter</button></div>' +
			'<div class="section" id="settingsDataSection">Game data</div>' +
			'<div class="hint" id="settingsDataText"></div>' +
			'<div class="buttons" id="settingsDataButtons"><button id="settingsDataDownload">Download</button><button id="settingsDataFile">From file</button>' +
			'<button id="settingsDataDelete">Delete</button><input type="file" id="settingsDataInput" hidden></div>' +
			'<div class="section">Stats</div>' +
			'<div class="hint" id="settingsAbout"></div>' +
			'<div class="buttons" id="settingsUpdateButtons"><button id="settingsUpdate">Get the newest version</button></div>' +
			'<div class="hint" id="settingsRestartHint">Restarts the game. Saves are kept, progress since the last save is lost.</div>' +
			'<div class="buttons"><button id="settingsRestart" class="accent">Restart</button><button id="settingsClose">Close</button></div>';
		document.body.appendChild( oPanel );

		var oStarted = {
			szResolution : oPageSettings.oValues.szResolution,
			szLowLatency : oPageSettings.oValues.szLowLatency
		};

		// The stored data pack: bStored is what's in storage now, the game uses what was there at the start
		var oData = { bStored : false, bChanged : false, szBusy : "" };

		function UpdateData()
		{
			var bSupported = oDataPack.IsSupported();
			document.getElementById( "settingsDataSection" ).style.display = bSupported ? "" : "none";
			document.getElementById( "settingsDataText" ).style.display    = bSupported ? "" : "none";
			document.getElementById( "settingsDataButtons" ).style.display = bSupported ? "" : "none";

			// The stored pack is found after the panel is made
			oData.bStored = oData.bStored || !!oDataPack.oPack;

			var szText;

			if ( oData.szBusy )
				szText = oData.szBusy;
			else if ( oData.bStored && oDataPack.oPack )
				szText = "Stored on this device (" + ( oDataPack.oPack.oFile.size / 1073741824 ).toFixed( 1 ) + " GB), the game loads from there";
			else if ( oData.bStored )
				szText = "Stored on this device, restart to use it";
			else
				szText = "Loaded from the server while you play. A copy on this device (about 1.7 GB) loads faster";

			document.getElementById( "settingsDataText" ).textContent = szText;

			var bBusy = !!oDataPack.oWorker;
			document.getElementById( "settingsDataDownload" ).style.display = oData.bStored ? "none" : "";
			document.getElementById( "settingsDataFile" ).style.display     = oData.bStored ? "none" : "";
			document.getElementById( "settingsDataDelete" ).style.display   = oData.bStored ? "" : "none";

			for ( var szId of [ "settingsDataDownload", "settingsDataFile", "settingsDataDelete" ] )
				document.getElementById( szId ).disabled = bBusy;
		}

		function StoreData( a_oMessage, a_szVerb )
		{
			oData.szBusy = a_szVerb + "...";
			UpdatePanel();

			oDataPack.Store( a_oMessage, function ( a_fProgress, a_iWritten ) {
				oData.szBusy = a_fProgress >= 0 ? a_szVerb + " " + Math.floor( a_fProgress * 100 ) + "%" : a_szVerb + ", " + Math.round( a_iWritten / 1048576 ) + " MB";
				UpdatePanel();
			} ).then( function () {
				oData.szBusy   = "";
				oData.bStored  = true;
				oData.bChanged = true;
				UpdatePanel();
			} ).catch( function ( a_oError ) {
				oData.szBusy = "Couldn't store the data: " + a_oError.message;
				UpdatePanel();
			} );
		}

		// The page's date shows whether an installed app got the newest files
		function UpdateAbout()
		{
			var szText = "Page from " + document.lastModified;
			var oContext = Module.oAudioContext;

			if ( oContext )
				szText += ". Sound " + oContext.sampleRate + " Hz, " + Math.round( ( Module.iAudioLag || 0 ) * 1000 / 44100 ) + " ms in the buffer (worst " +
					Math.round( ( Module.iAudioWorst || 0 ) * 1000 / 44100 ) + " ms), " + Math.round( ( ( oContext.baseLatency || 0 ) + ( oContext.outputLatency || 0 ) ) * 1000 ) + " ms output, " +
					( Module.iAudioGaps || 0 ) + " gaps, " + ( Module.iAudioSkips || 0 ) + " skips" +
					( Module._WASM_GetAudioWorstStart ? ", sounds start within " + Math.round( Module._WASM_GetAudioWorstStart() ) + " ms" : "" );

			document.getElementById( "settingsAbout" ).textContent = szText;
			document.getElementById( "settingsUpdateButtons" ).style.display = navigator.serviceWorker && !/[?&]app(&|$)/.test( location.search ) ? "" : "none";

			// Offline starts need the service worker running this page, the page files it keeps and the stored data
			if ( navigator.serviceWorker && window.caches && !/[?&]app(&|$)/.test( location.search ) )
			{
				GetOfflineState().then( function ( a_szState ) {
					document.getElementById( "settingsAbout" ).textContent = szText + ". " + a_szState;
				} );
			}
		}

		async function GetOfflineState()
		{
			try
			{
				if ( !navigator.serviceWorker.controller )
					return "Offline: not ready, open the game again while online";

				var iKept = 0;

				for ( var szName of await caches.keys() )
				{
					if ( szName.indexOf( "barnyard-" ) == 0 && await ( await caches.open( szName ) ).match( new URL( "OpenBarnyard.wasm", location.href ).href ) )
						iKept++;
				}

				if ( !iKept )
					return "Offline: not ready, the page files aren't kept yet";

				return oData.bStored ? "Offline: ready" : "Offline: needs the game data (Download above)";
			}
			catch ( e )
			{
				return "Offline: unknown (" + e.message + ")";
			}
		}

		function UpdatePanel()
		{
			document.getElementById( "settingsTouchRow" ).style.display = document.getElementById( "touch" ) ? "" : "none";
			UpdateData();
			UpdateAbout();

			var bRestart = oStarted.szResolution != oPageSettings.oValues.szResolution || oStarted.szLowLatency != oPageSettings.oValues.szLowLatency || oData.bChanged;

			document.getElementById( "settingsRestart" ).style.display     = bRestart ? "" : "none";
			document.getElementById( "settingsRestartHint" ).style.display = bRestart ? "" : "none";
			document.getElementById( "settingsAuto" ).textContent = "Now " + Module.oCanvas.width + "x" + Module.oCanvas.height + ( oPageSettings.IsSlowDevice() ? ", auto treats this device as slow" : "" ) +
				( Module.oAudioContext && Module.oAudioContext.state != "running" ? ", sound is " + Module.oAudioContext.state : "" );
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

		document.getElementById( "settingsDataDownload" ).addEventListener( "click", function () {
			StoreData( { szUrl : oDataPack.GetPackUrl() }, "Downloading" );
		} );

		document.getElementById( "settingsDataFile" ).addEventListener( "click", function () {
			document.getElementById( "settingsDataInput" ).click();
		} );

		// A .tar or .tar.gz of the Data folder
		document.getElementById( "settingsDataInput" ).addEventListener( "change", function ( a_oEvent ) {
			var oFile = a_oEvent.target.files[ 0 ];
			a_oEvent.target.value = "";

			if ( oFile )
				StoreData( { oFile : oFile }, "Copying" );
		} );

		// The running game reads the stored copy, so it restarts right away
		document.getElementById( "settingsDataDelete" ).addEventListener( "click", function () {
			if ( !confirm( "Delete the game data stored on this device?" ) )
				return;

			oDataPack.Store( { bDelete : true }, function () {} ).then( function () {
				location.reload();
			} ).catch( function ( a_oError ) {
				oData.szBusy = "Couldn't delete the data: " + a_oError.message;
				UpdatePanel();
			} );
		} );

		// Drops the kept page files (not the game data) and loads them again from the server
		document.getElementById( "settingsUpdate" ).addEventListener( "click", async function () {
			if ( !navigator.onLine )
			{
				document.getElementById( "settingsAbout" ).textContent = "Getting the newest version needs a connection";
				return;
			}

			try
			{
				for ( var oRegistration of await navigator.serviceWorker.getRegistrations() )
					await oRegistration.unregister();

				for ( var szName of await caches.keys() )
				{
					if ( szName.indexOf( "barnyard-" ) == 0 )
						await caches.delete( szName );
				}
			}
			catch ( e )
			{
			}

			location.reload();
		} );

		document.getElementById( "settingsTouchLayout" ).addEventListener( "click", function () {
			oPanel.style.display = "none";
			oGameInput.EditTouchLayout();
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
