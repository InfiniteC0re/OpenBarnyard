#include "ToshiPCH.h"
#include "TFile.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

    TFile::TFile( TFileSystem* pFS )
    {
        m_pFileSystem = pFS;
    }

    TFile::TFile( const TFile& other )
    {
        m_pFileSystem = other.GetFileSystem();
    }

    TFile* TFile::Create( const TString8& filename, TFILEMODE mode )
    {
        TASSERT( filename.IsIndexValid( 0 ), "filename is empty" );

        TFile* pFile = TFileManager::GetSingletonSafe()->CreateFile( filename, mode );

        if ( pFile != TNULL )
        {
            TTRACE( "TFile::Create - Created file %s with mode %hu\n", filename.GetString(), mode );
        }
        else
        {
            TTRACE( "TFile::Create - Failed to create file %s with mode %hu\n", filename.GetString(), mode );
        }

        return pFile;
    }

    TString8 TFile::ConcatPath( const TString8& a_rcPath1, const TString8& a_rcPath2 )
    {
        TString8 strResult;

        TINT iPath1StartPos = 0;
        TINT iPath2StartPos = 0;

        // Copy filesystem/volume name from a_rcPath1 or a_rcPath2
        // prioritizing the one in a_rcPath2
        TINT iColonPos = a_rcPath2.FindReverse( ':' );
        
        if ( iColonPos == -1 )
        {
            iColonPos = a_rcPath1.FindReverse( ':' );

            if ( iColonPos > -1 )
            {
                // Copy volume/filesystem name from the first path
                iPath1StartPos = iColonPos + 1;
                strResult.Concat( a_rcPath1, iPath1StartPos );
            }
        }
        else
        {
            // Copy volume/filesystem name from the second path
            iPath2StartPos = iColonPos + 1;
            strResult.Concat( a_rcPath2, iPath2StartPos );

            // Ignore everything before the ':' character in the first path
            iPath1StartPos = a_rcPath1.FindReverse( ':' ) + 1;
        }

        TINT iPath1Length = a_rcPath1.Length();
        TINT iPath2Length = a_rcPath2.Length();

        if ( iPath2StartPos < iPath2Length &&
             ( a_rcPath2[ iPath2StartPos ] == '/' || a_rcPath2[ iPath2StartPos ] == '\\' ) )
        {
            // The second path starts from a filesystem/volume root so ignore the first path
            strResult += a_rcPath2.GetString( iPath2StartPos );
        }
        else
        {
            // Concat the first path with the second one trimming parts like 'C:' or 'native:' and etc.
            strResult += a_rcPath1.GetString( iPath1StartPos );
            if ( iPath1StartPos < iPath1Length && iPath2StartPos < iPath2Length &&
                 a_rcPath1[ iPath1Length - 1 ] != '/' && a_rcPath1[ iPath1Length - 1 ] != '\\' )
            {
                strResult += "/";
            }

            strResult += a_rcPath2.GetString( iPath2StartPos );
        }

        return SimplifyPath( strResult );
    }

    TString8 TFile::SimplifyPath( const TString8& a_rcPath )
    {
        TString8 strPath = a_rcPath;
        FixPathSlashesBack( strPath );

        TINT iPos = 0;
        TString8 strResult;
        
        // Keep filesystem/volume/protocol name in the simplified path and leave it as it is
        TINT iColonPos = strPath.FindReverse( ':' );
        if ( iColonPos > -1 )
        {
            iPos = iColonPos + 1;
            strResult.Concat( strPath, iPos );
        }

        if ( strPath[ iPos ] == '/' )
        {
            iPos += 1;
            strResult += "/";
        }

        TINT iInitialResultLength = strResult.Length();
        TINT iResultLength = iInitialResultLength;
        
        strPath += "/";
        TINT iSlashPos = strPath.Find( '/', iPos );

        do 
        {
            // Is the path over?
            if ( iSlashPos < 0 )
            {
                if ( iInitialResultLength < strResult.Length() )
                {
                    strResult.Truncate( strResult.Length() - 1 );
                }

                return strResult;
            }

            if ( !strncmp( strPath.GetString( iPos ), "../", 3 ) )
            {
                if ( iResultLength == strResult.Length() )
                {
                    // Can't go back to a previous dir since it's the only one
                    strResult += "../";
                    iResultLength += 3;
                }
                else
                {
                    // Truncate the path removing the current dir
                    TINT iTruncateTo;
                    
                    if ( strResult.Length() > 1 && ( iTruncateTo = strResult.FindReverse( '/', strResult.Length() - 2 ), iResultLength <= iTruncateTo ) )
                    {
                        strResult.Truncate( iTruncateTo + 1 );
                    }
                    else
                    {
                        strResult.Truncate( iResultLength );
                    }
                }
            }
            // Add anything except './'
            else if ( strncmp( strPath.GetString( iPos ), "./", 2 ) != 0 )
            {
                strResult.Concat( strPath.GetString( iPos ), ( iSlashPos - iPos ) + 1 );
            }

            iPos = iSlashPos + 1;
            iSlashPos = strPath.Find( '/', iPos );

        } while ( TTRUE );
    }

}
