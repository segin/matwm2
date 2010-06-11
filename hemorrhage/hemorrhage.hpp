/* hemorrhage.hpp: Main header for Hemorrhage. 
 * Copyright (c) Year(s), Company or Person's Name <E-mail address>
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
	
	class Hemorrhage { 
		public:
