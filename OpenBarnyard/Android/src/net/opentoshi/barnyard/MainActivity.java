package net.opentoshi.barnyard;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.webkit.ConsoleMessage;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Set;
import java.util.concurrent.atomic.AtomicReference;

import dalvik.system.DexFile;

public class MainActivity extends Activity
{
	private WebView m_oWebView;
	private LocalServer m_oServer;

	// How the page gets shared memory, debug builds show it to the page
	private static String s_szSharedMemoryWay = "none";

	@Override
	protected void onCreate( Bundle a_oState )
	{
		super.onCreate( a_oState );

		getWindow().addFlags( WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON );
		if ( Build.VERSION.SDK_INT >= 28 )
			getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;

		allowSharedMemoryOnOldWebView();

		WebView.setWebContentsDebuggingEnabled( ( getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE ) != 0 );

		m_oWebView = new WebView( this );
		setContentView( m_oWebView );

		WebSettings oSettings = m_oWebView.getSettings();
		oSettings.setJavaScriptEnabled( true );
		oSettings.setDomStorageEnabled( true );
		oSettings.setDatabaseEnabled( true );
		oSettings.setMediaPlaybackRequiresUserGesture( false );
		oSettings.setCacheMode( WebSettings.LOAD_NO_CACHE );

		m_oWebView.setWebChromeClient( new WebChromeClient()
		{
			@Override
			public boolean onConsoleMessage( ConsoleMessage a_oMessage )
			{
				return true;
			}
		} );

		try
		{
			m_oServer = new LocalServer( new File( getApplicationInfo().sourceDir ), getPackageName().equals( "net.opentoshi.barnyard" ) ? LocalServer.GAME_PORT : LocalServer.GAME_PORT + 1 );
			m_oServer.start();

			allowCrossOriginIsolation( m_oServer.getOrigin() );

			// Without a client every navigation, even a reload, goes to the browser. The game's pages stay here
			final String szOrigin = m_oServer.getOrigin();

			m_oWebView.setWebViewClient( new WebViewClient()
			{
				@Override
				public boolean shouldOverrideUrlLoading( WebView a_oView, WebResourceRequest a_oRequest )
				{
					// Quit in the game's menu
					if ( "barnyard".equals( a_oRequest.getUrl().getScheme() ) )
					{
						finish();
						return true;
					}

					if ( a_oRequest.getUrl().toString().startsWith( szOrigin ) )
						return false;

					// Links in the page open in the browser
					String szScheme = a_oRequest.getUrl().getScheme();

					if ( "https".equals( szScheme ) || "http".equals( szScheme ) )
					{
						try
						{
							startActivity( new Intent( Intent.ACTION_VIEW, a_oRequest.getUrl() ) );
						}
						catch ( Exception e )
						{
						}
					}

					return true;
				}
			} );

			String szUrl = m_oServer.getUrl() + "?touch&nosw&fps&app";

			// Debug builds can be started with game arguments: adb shell am start -n <activity> --es args -simstate
			String szArgs = getIntent().getStringExtra( "args" );

			if ( ( getApplicationInfo().flags & ApplicationInfo.FLAG_DEBUGGABLE ) != 0 )
			{
				if ( szArgs != null )
					szUrl += "&args=" + URLEncoder.encode( szArgs, "UTF-8" );

				szUrl += "&way=" + URLEncoder.encode( s_szSharedMemoryWay, "UTF-8" ) + "&sdk=" + Build.VERSION.SDK_INT;
			}

			m_oWebView.loadUrl( szUrl );
		}
		catch ( IOException e )
		{
			m_oWebView.loadData( "<body style='background:#000;color:#fff;font:18px sans-serif;padding:24px'>Couldn't start: " + e, "text/html", "utf-8" );
		}
	}

	// WebViews without the allow list below have no API for this, only the engine's own switches.
	// They have to be set before the engine starts, and the class has a different name in every version:
	// WV.* in Google's WebView, a short top level name in Huawei's.
	private static void allowSharedMemoryOnOldWebView()
	{
		try
		{
			ClassLoader oFilter = WebView.getWebViewClassLoader();

			if ( hasCrossOriginIsolationAllowList( oFilter ) )
				return;

			// Google's loader filters what the app sees, the one behind it has everything. Others give that one directly
			ClassLoader[] aLoaders = { oFilter.getClass().getClassLoader(), oFilter };
			ArrayList<String> aNames = new ArrayList<>();

			// The names of the WebView's classes: from its APK, from the dex files its loader really uses (a preinstalled
			// WebView keeps them out of the APK), at last every one or two letter name (the obfuscated ones are that short)
			String szApk = addApkNames( aNames );
			String szFound = tryNames( aNames, aLoaders );

			int iLoaderNames = 0;

			if ( szFound == null )
			{
				aNames.clear();

				for ( ClassLoader oLoader : aLoaders )
					addLoaderNames( aNames, oLoader );

				iLoaderNames = aNames.size();
				szFound = tryNames( aNames, aLoaders );
			}

			if ( szFound == null )
			{
				aNames.clear();
				addShortNames( aNames );
				szFound = tryNames( aNames, aLoaders );
			}

			if ( szFound != null )
			{
				Log.i( "Barnyard", "Set the WebView switches with " + szFound );
				s_szSharedMemoryWay = "switches " + szFound;
				return;
			}

			s_szSharedMemoryWay = "none, no switches found (APK " + szApk + ", loader " + iLoaderNames + " names, " + aLoaders[ 0 ] + ", " + s_szSearchError + ")";
			Log.e( "Barnyard", s_szSharedMemoryWay );
		}
		catch ( Throwable e )
		{
			Log.e( "Barnyard", "Couldn't set the WebView switch", e );
			s_szSharedMemoryWay = "none, " + e;
		}
	}

	// The first class that loaded but couldn't be used
	private static String s_szSearchError = "no errors";

	// The name of the class the switches were set with, or null
	private static String tryNames( ArrayList<String> a_aNames, ClassLoader[] a_aLoaders )
	{
		for ( String szName : a_aNames )
		{
			if ( !szName.startsWith( "WV." ) && szName.indexOf( '.' ) >= 0 && !szName.equals( "org.chromium.base.CommandLine" ) )
				continue;

			for ( ClassLoader oLoader : a_aLoaders )
			{
				Class<?> oClass;

				try
				{
					oClass = Class.forName( szName, false, oLoader );
				}
				catch ( Throwable e )
				{
					continue;
				}

				try
				{
					if ( appendEngineSwitches( oClass ) )
						return szName;
				}
				catch ( Throwable e )
				{
					if ( s_szSearchError.equals( "no errors" ) )
						s_szSearchError = szName + ": " + e;
				}
			}
		}

		return null;
	}

	// How many names there were, or why there were none
	private static String addApkNames( ArrayList<String> a_aNames )
	{
		try
		{
			DexFile oDex = new DexFile( WebView.getCurrentWebViewPackage().applicationInfo.sourceDir );

			try
			{
				for ( Enumeration<String> oNames = oDex.entries(); oNames.hasMoreElements(); )
					a_aNames.add( oNames.nextElement() );
			}
			finally
			{
				oDex.close();
			}

			return a_aNames.size() + " names";
		}
		catch ( Throwable e )
		{
			return e.toString();
		}
	}

	private static void addLoaderNames( ArrayList<String> a_aNames, ClassLoader a_oLoader )
	{
		try
		{
			Field oPathList = Class.forName( "dalvik.system.BaseDexClassLoader" ).getDeclaredField( "pathList" );
			oPathList.setAccessible( true );
			Object oList = oPathList.get( a_oLoader );

			Field oElements = oList.getClass().getDeclaredField( "dexElements" );
			oElements.setAccessible( true );

			for ( Object oElement : ( Object[] )oElements.get( oList ) )
			{
				Field oDexFile = oElement.getClass().getDeclaredField( "dexFile" );
				oDexFile.setAccessible( true );
				DexFile oDex = ( DexFile )oDexFile.get( oElement );

				if ( oDex != null )
				{
					for ( Enumeration<String> oNames = oDex.entries(); oNames.hasMoreElements(); )
						a_aNames.add( oNames.nextElement() );
				}
			}
		}
		catch ( Throwable e )
		{
			// Not a dex loader, or its insides are hidden
		}
	}

	private static void addShortNames( ArrayList<String> a_aNames )
	{
		final String szLetters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		final String szSecond  = szLetters + "0123456789";

		for ( int i = 0; i < szLetters.length(); i++ )
		{
			a_aNames.add( szLetters.substring( i, i + 1 ) );

			for ( int j = 0; j < szSecond.length(); j++ )
				a_aNames.add( "" + szLetters.charAt( i ) + szSecond.charAt( j ) );
		}
	}

	private static boolean hasCrossOriginIsolationAllowList( ClassLoader a_oLoader )
	{
		try
		{
			Class.forName( "org.chromium.support_lib_boundary.ProfileBoundaryInterface", false, a_oLoader ).getMethod( "setCrossOriginIsolatedAllowList", Set.class );
			return true;
		}
		catch ( Throwable e )
		{
			return false;
		}
	}

	// Looks for org.chromium.base.CommandLine by its shape: its instance is in a static field of its own type or an
	// AtomicReference. The WebView sets it up when it's loaded, before the engine
	private static boolean appendEngineSwitches( Class<?> a_oClass ) throws ReflectiveOperationException
	{
		Object oCommandLine = null;

		for ( Field oField : a_oClass.getDeclaredFields() )
		{
			if ( !Modifier.isStatic( oField.getModifiers() ) )
				continue;

			if ( oField.getType() == a_oClass )
			{
				oField.setAccessible( true );
				oCommandLine = oField.get( null );
			}
			else if ( oField.getType() == AtomicReference.class )
			{
				oField.setAccessible( true );
				Object oValue = ( ( AtomicReference<?> )oField.get( null ) ).get();

				if ( a_oClass.isInstance( oValue ) )
					oCommandLine = oValue;
			}
		}

		if ( oCommandLine == null )
			return false;

		// Java's (a map and a list) or the native one the WebView switches to once its library is loaded (Huawei's does
		// that before the engine starts). Both have appendSwitchWithValue, hasSwitch and getCommandLineArguments
		Method oAppend = null;
		boolean bHasSwitch = false;
		boolean bArguments = false;

		for ( Class<?> oClass = oCommandLine.getClass(); oClass != null && oClass != Object.class; oClass = oClass.getSuperclass() )
		{
			for ( Method oMethod : oClass.getDeclaredMethods() )
			{
				if ( Modifier.isStatic( oMethod.getModifiers() ) || Modifier.isAbstract( oMethod.getModifiers() ) )
					continue;

				Class<?>[] aParams = oMethod.getParameterTypes();
				Class<?>   oReturn = oMethod.getReturnType();

				if ( oAppend == null && oReturn == void.class && aParams.length == 2 && aParams[ 0 ] == String.class && aParams[ 1 ] == String.class )
					oAppend = oMethod;
				else if ( oReturn == boolean.class && aParams.length == 1 && aParams[ 0 ] == String.class )
					bHasSwitch = true;
				else if ( oReturn == String[].class && aParams.length == 0 )
					bArguments = true;
			}
		}

		if ( oAppend == null || !bHasSwitch || !bArguments )
			return false;

		// Before the isolation rules shared memory was a Blink feature, older engines lift the rule with the second one
		oAppend.setAccessible( true );
		oAppend.invoke( oCommandLine, "enable-blink-features", "SharedArrayBuffer,UnrestrictedSharedArrayBuffer" );
		oAppend.invoke( oCommandLine, "enable-features", "SharedArrayBuffer" );
		return true;
	}

	// The game's threads need a cross-origin isolated page, WebView only allows that for origins on this list.
	// AndroidX has this as Profile.setCrossOriginIsolatedAllowlist, this is the same call without the library.
	private static void allowCrossOriginIsolation( String a_szOrigin )
	{
		try
		{
			ClassLoader oLoader = WebView.getWebViewClassLoader();
			String szBoundary = "org.chromium.support_lib_boundary.";

			InvocationHandler oFactory = ( InvocationHandler )Class.forName( "org.chromium.support_lib_glue.SupportLibReflectionUtil", false, oLoader ).getDeclaredMethod( "createWebViewProviderFactory" ).invoke( null );
			InvocationHandler oStore = ( InvocationHandler )oFactory.invoke( null, Class.forName( szBoundary + "WebViewProviderFactoryBoundaryInterface", false, oLoader ).getMethod( "getProfileStore" ), null );
			InvocationHandler oProfile = ( InvocationHandler )oStore.invoke( null, Class.forName( szBoundary + "ProfileStoreBoundaryInterface", false, oLoader ).getMethod( "getProfile", String.class ), new Object[] { "Default" } );

			oProfile.invoke( null, Class.forName( szBoundary + "ProfileBoundaryInterface", false, oLoader ).getMethod( "setCrossOriginIsolatedAllowList", Set.class ), new Object[] { Collections.singleton( a_szOrigin ) } );
			s_szSharedMemoryWay = "allow list";
		}
		catch ( Throwable e )
		{
			Log.e( "Barnyard", "Couldn't allow cross-origin isolation", e );
		}
	}

	@Override
	public void onWindowFocusChanged( boolean a_bHasFocus )
	{
		super.onWindowFocusChanged( a_bHasFocus );

		if ( a_bHasFocus && Build.VERSION.SDK_INT >= 30 )
		{
			WindowInsetsController oInsets = getWindow().getInsetsController();

			if ( oInsets != null )
			{
				oInsets.hide( WindowInsets.Type.systemBars() );
				oInsets.setSystemBarsBehavior( WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE );
			}
		}
	}

	// The back button is the game's Escape
	@Override
	public boolean dispatchKeyEvent( KeyEvent a_oEvent )
	{
		if ( a_oEvent.getKeyCode() == KeyEvent.KEYCODE_BACK )
		{
			m_oWebView.dispatchKeyEvent( new KeyEvent( a_oEvent.getAction(), KeyEvent.KEYCODE_ESCAPE ) );
			return true;
		}

		return super.dispatchKeyEvent( a_oEvent );
	}

	@Override
	protected void onPause()
	{
		super.onPause();
		m_oWebView.onPause();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		m_oWebView.onResume();
	}

	@Override
	protected void onDestroy()
	{
		if ( m_oServer != null )
			m_oServer.stop();

		m_oWebView.destroy();
		super.onDestroy();
	}
}
