/* hemorrhage.hpp: Main header for Hemorrhage. 
 *
 * Copyright (c) 2010, Kirn Gill <segin2005@gmail.com>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include <boost/asio.hpp>
#include <iostream>
#include <string>
using namespace std;

/*
Type LastFM Alias "LastFM"
Private:
	m_artist(500) As String
	m_name(500) As String
	m_album(500) As String
	m_length(500) As UInteger
	m_curtime(500) As Integer
	m_entries As Integer
	m_session As String
	c_username As String
	c_password As String
	c_apihost(2) As String
	c_apiport(2) As Short
	c_apipath(2) As String
	c_xmlpath As String
Public:
	Declare Constructor()
	Declare Destructor()
	Declare Sub readConfig Alias "readConfig" ()
	Declare Function getSessionKey Alias "getSessionKey" () As String
	Declare Function setNowPlaying Alias "setNowPlaying" () As SOCKET
	Declare Function scrobbleTrack Alias "scrobbleTrack" () As Integer
	Declare Function submitData Alias "submitData" (sData As String, host As Integer) As SOCKET
	Declare Function submitScrobble Alias "submitScrobble" (artist As String, Title As String, album As String, length As Integer, curtime As UInteger) As Integer
	Declare Function saveScrobble Alias "saveScrobble" (artist As String, Title As String, album As String, length As Integer, curtime As UInteger) As Integer
	Declare Sub dumpScrobbles Alias "dumpScrobbles" ()
	Declare Sub dumpScrobbles2 Alias "dumpScrobbles2" () 
	Declare Sub loadScrobbles Alias "loadScrobbles" ()
	Declare Sub submitSavedScrobbles Alias "submitSavedScrobbles" ()
End Type
*/

namespace Hemorrhage {

	class Scrobble { 
		private: 
			string m_artist;
			string m_title;
			string m_album;
			unsigned int m_length;
			unsigned int m_curtime;
		public: 
			Scrobble();
			Scrobble(string artist, string title, string album, unsigned int length, unsigned int curtime);
			~Scrobble();
			void setScrobbleData(string artist, string title, string album, unsigned int length, unsigned int curtime);\
			void fb_setScrobbleData(const char *artist, const char *title, const char *album, unsigned int length, unsigned int curtime);
	};

	class Scrobbler { 
		private:
			Scrobble m_scrobbles[500];
			string	m_session;
			string	c_username;
			string	c_password;
			string	c_apihost[2];
			short 	c_apiport[2];
			string	c_apipath[2];
			string	c_xmlpath;
		public:
	
	};
	
}
