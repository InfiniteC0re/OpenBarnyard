package net.opentoshi.barnyard;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URLDecoder;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;

// Serves the page and the game from the assets of the APK, with the headers threads need and Range requests.
// All of them are stored uncompressed, so they are read straight from the APK file.
// The port is fixed: the saves are in IndexedDB, which belongs to the address.
final class LocalServer
{
	// Other packages (the WebView test) take the next one, so both can run
	static final int GAME_PORT = 17263;

	private final int m_iPort;

	private static final class Entry
	{
		long uiHeader;
		long uiData = -1;
		long uiSize;
	}

	private final HashMap<String, Entry> m_oEntries = new HashMap<>();
	private final FileChannel m_oApk;
	private ServerSocket m_oSocket;

	LocalServer( File a_oApk, int a_iPort ) throws IOException
	{
		m_iPort = a_iPort;
		m_oApk = new RandomAccessFile( a_oApk, "r" ).getChannel();
		readEntries();
	}

	private ByteBuffer read( long a_uiPosition, int a_iSize ) throws IOException
	{
		ByteBuffer oBuffer = ByteBuffer.allocate( a_iSize ).order( ByteOrder.LITTLE_ENDIAN );

		while ( oBuffer.hasRemaining() )
		{
			if ( m_oApk.read( oBuffer, a_uiPosition + oBuffer.position() ) <= 0 )
				throw new IOException( "The APK ended early" );
		}

		oBuffer.flip();
		return oBuffer;
	}

	// The file list at the end of the zip
	private void readEntries() throws IOException
	{
		final long uiSize = m_oApk.size();
		final int iTail = ( int )Math.min( uiSize, 66000 );
		ByteBuffer oTail = read( uiSize - iTail, iTail );

		int iEnd = iTail - 22;

		while ( iEnd >= 0 && oTail.getInt( iEnd ) != 0x06054b50 )
			iEnd--;

		if ( iEnd < 0 )
			throw new IOException( "The APK has no zip directory" );

		final int iNumEntries = oTail.getShort( iEnd + 10 ) & 0xFFFF;
		final long uiDirectorySize = oTail.getInt( iEnd + 12 ) & 0xFFFFFFFFL;
		final long uiDirectory = oTail.getInt( iEnd + 16 ) & 0xFFFFFFFFL;

		ByteBuffer oDirectory = read( uiDirectory, ( int )uiDirectorySize );
		int iAt = 0;

		for ( int i = 0; i < iNumEntries; i++ )
		{
			final int iNameLength = oDirectory.getShort( iAt + 28 ) & 0xFFFF;
			final int iExtraLength = oDirectory.getShort( iAt + 30 ) & 0xFFFF;
			final int iCommentLength = oDirectory.getShort( iAt + 32 ) & 0xFFFF;

			// aapt2 on Windows writes the separators of nested assets as backslashes
			String szName = new String( oDirectory.array(), iAt + 46, iNameLength, StandardCharsets.UTF_8 ).replace( '\\', '/' );

			if ( szName.startsWith( "assets/" ) && oDirectory.getShort( iAt + 10 ) == 0 )
			{
				Entry oEntry = new Entry();
				oEntry.uiSize = oDirectory.getInt( iAt + 24 ) & 0xFFFFFFFFL;
				oEntry.uiHeader = oDirectory.getInt( iAt + 42 ) & 0xFFFFFFFFL;
				m_oEntries.put( szName.substring( 7 ), oEntry );
			}

			iAt += 46 + iNameLength + iExtraLength + iCommentLength;
		}
	}

	private long getDataPosition( Entry a_oEntry ) throws IOException
	{
		if ( a_oEntry.uiData < 0 )
		{
			ByteBuffer oHeader = read( a_oEntry.uiHeader, 30 );
			a_oEntry.uiData = a_oEntry.uiHeader + 30 + ( oHeader.getShort( 26 ) & 0xFFFF ) + ( oHeader.getShort( 28 ) & 0xFFFF );
		}

		return a_oEntry.uiData;
	}

	String getUrl()
	{
		return getOrigin() + "/";
	}

	String getOrigin()
	{
		return "http://127.0.0.1:" + m_iPort;
	}

	void start() throws IOException
	{
		m_oSocket = new ServerSocket( m_iPort, 16, InetAddress.getByName( "127.0.0.1" ) );

		Thread oThread = new Thread( this::acceptLoop, "barnyard-server" );
		oThread.setDaemon( true );
		oThread.start();
	}

	void stop()
	{
		try
		{
			m_oSocket.close();
		}
		catch ( IOException e )
		{
		}
	}

	private void acceptLoop()
	{
		while ( !m_oSocket.isClosed() )
		{
			try
			{
				Socket oClient = m_oSocket.accept();
				oClient.setTcpNoDelay( true );

				Thread oThread = new Thread( () -> serve( oClient ), "barnyard-client" );
				oThread.setDaemon( true );
				oThread.start();
			}
			catch ( IOException e )
			{
				return;
			}
		}
	}

	private void serve( Socket a_oClient )
	{
		try ( Socket oClient = a_oClient )
		{
			InputStream oIn = new BufferedInputStream( oClient.getInputStream() );
			OutputStream oOut = new BufferedOutputStream( oClient.getOutputStream(), 65536 );

			while ( handleRequest( oIn, oOut ) )
				oOut.flush();

			oOut.flush();
		}
		catch ( IOException e )
		{
		}
	}

	private static String readLine( InputStream a_oIn ) throws IOException
	{
		ByteArrayOutputStream oLine = new ByteArrayOutputStream();

		for ( int c = a_oIn.read(); c != '\n'; c = a_oIn.read() )
		{
			if ( c < 0 )
				return null;

			if ( c != '\r' )
				oLine.write( c );
		}

		return oLine.toString( "UTF-8" );
	}

	private boolean handleRequest( InputStream a_oIn, OutputStream a_oOut ) throws IOException
	{
		String szRequest = readLine( a_oIn );

		if ( szRequest == null || szRequest.isEmpty() )
			return false;

		String szRange = null;

		for ( String szHeader = readLine( a_oIn ); szHeader != null && !szHeader.isEmpty(); szHeader = readLine( a_oIn ) )
		{
			if ( szHeader.regionMatches( true, 0, "Range:", 0, 6 ) )
				szRange = szHeader.substring( 6 ).trim();
		}

		String[] aParts = szRequest.split( " " );

		if ( aParts.length < 2 )
			return false;

		boolean bHead = aParts[ 0 ].equals( "HEAD" );
		String szPath = aParts[ 1 ];
		int iQuery = szPath.indexOf( '?' );

		if ( iQuery >= 0 )
			szPath = szPath.substring( 0, iQuery );

		// '+' is a real character in a path
		szPath = URLDecoder.decode( szPath.replace( "+", "%2B" ), "UTF-8" );

		if ( szPath.contains( ".." ) )
			return sendEmpty( a_oOut, "403 Forbidden" );

		String szAsset = szPath.equals( "/" ) ? "index.html" : szPath.substring( 1 );

		return sendAsset( a_oOut, szAsset, szRange, bHead );
	}

	private static void writeHeaders( OutputStream a_oOut, String a_szStatus, String a_szType, long a_uiLength, String a_szExtra ) throws IOException
	{
		String szHeaders = "HTTP/1.1 " + a_szStatus + "\r\n" +
			"Cross-Origin-Opener-Policy: same-origin\r\n" +
			"Cross-Origin-Embedder-Policy: require-corp\r\n" +
			"Cross-Origin-Resource-Policy: same-origin\r\n" +
			"Document-Isolation-Policy: isolate-and-require-corp\r\n" +
			"Accept-Ranges: bytes\r\n" +
			"Cache-Control: no-store\r\n" +
			"Connection: keep-alive\r\n" +
			"Content-Type: " + a_szType + "\r\n" +
			"Content-Length: " + a_uiLength + "\r\n" +
			a_szExtra + "\r\n";

		a_oOut.write( szHeaders.getBytes( StandardCharsets.US_ASCII ) );
	}

	private static boolean sendEmpty( OutputStream a_oOut, String a_szStatus ) throws IOException
	{
		writeHeaders( a_oOut, a_szStatus, "text/plain", 0, "" );
		return true;
	}

	private boolean sendAsset( OutputStream a_oOut, String a_szName, String a_szRange, boolean a_bHead ) throws IOException
	{
		Entry oEntry = m_oEntries.get( a_szName );

		if ( oEntry == null )
			return sendEmpty( a_oOut, "404 Not Found" );

		final long uiSize = oEntry.uiSize;
		long uiStart = 0;
		long uiEnd = uiSize - 1;
		boolean bPartial = false;

		if ( a_szRange != null && a_szRange.startsWith( "bytes=" ) )
		{
			String[] aRange = a_szRange.substring( 6 ).split( "-", -1 );

			if ( !aRange[ 0 ].isEmpty() )
			{
				uiStart  = Long.parseLong( aRange[ 0 ] );
				bPartial = true;

				if ( aRange.length > 1 && !aRange[ 1 ].isEmpty() )
					uiEnd = Math.min( Long.parseLong( aRange[ 1 ] ), uiSize - 1 );
			}
		}

		final long uiLength = Math.max( uiEnd - uiStart + 1, 0 );

		if ( bPartial )
			writeHeaders( a_oOut, "206 Partial Content", getType( a_szName ), uiLength, "Content-Range: bytes " + uiStart + "-" + uiEnd + "/" + uiSize + "\r\n" );
		else
			writeHeaders( a_oOut, "200 OK", getType( a_szName ), uiLength, "" );

		if ( a_bHead )
			return true;

		long uiPosition = getDataPosition( oEntry ) + uiStart;
		long uiLeft = uiLength;
		byte[] aChunk = new byte[ 65536 ];

		while ( uiLeft > 0 )
		{
			ByteBuffer oBuffer = ByteBuffer.wrap( aChunk, 0, ( int )Math.min( uiLeft, aChunk.length ) );
			int iRead = m_oApk.read( oBuffer, uiPosition );

			if ( iRead <= 0 )
				return false;

			a_oOut.write( aChunk, 0, iRead );
			uiPosition += iRead;
			uiLeft -= iRead;
		}

		return true;
	}

	private static String getType( String a_szName )
	{
		if ( a_szName.endsWith( ".html" ) ) return "text/html";
		if ( a_szName.endsWith( ".js" ) ) return "text/javascript";
		if ( a_szName.endsWith( ".wasm" ) ) return "application/wasm";
		if ( a_szName.endsWith( ".txt" ) ) return "text/plain";
		return "application/octet-stream";
	}
}
