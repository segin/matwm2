/* hemorrhage.cpp: Main soruce file of Hemorrhage. 
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
 
#include "hemorrhage.hpp"

Hemorrhage::Scrobble::Scrobble() 
{
	this->m_length = 0;
	this->m_curtime = 0;
}

Hemorrhage::Scrobble::~Scrobble() 
{
	this->m_length = 0;
	this->m_curtime = 0;
}

Hemorrhage::Scrobble::Scrobble(string artist, string title, string album, unsigned int length, unsigned int curtime)
{
	setScrobbleData(artist, title, album, length, curtime);
}

void Hemorrhage::Scrobble::setScrobbleData(string artist, string title, string album, unsigned int length, unsigned int curtime)
{
	this->m_artist = artist;
	this->m_title = title;
	this->m_album = album;
	this->m_length = length;
	this->m_curtime = curtime;
}

void Hemorrhage::Scrobble::fb_setScrobbleData(const char *artist, const char *title, const char *album, unsigned int length, unsigned int curtime)
{
	setScrobbleData(artist, title, album, length, curtime);
}


Hemorrhage::Scrobbler::Scrobbler()
{
	std::cout << "Fuck me." << std::endl;
}
