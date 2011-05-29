/*
 * Copyright (c) 2011, Kirn Gill <segin2005@gmail.com>
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

package com.segin.LastFM;

import java.io.*;
import java.util.Date;
import java.security.*;
import java.math.BigInteger;
import java.net.URLEncoder;

class Scrobbler {
	String sessionkey = "";
	String username = "";
	String password = "";

	public Scrobbler() { 
		readConfig("lastfm_config.txt");
		System.out.println("lastfm username: " + username + ", password: " + doAskterisks(password));
		System.out.println(authKey(password));
		getSessionKey();
	}
	
	public static void main(String[] args) {
		long sec;
		Date date;
		if(args.length < 3) { 
			System.out.println("Error: not enough args!");
			return;
		}
		try {
			Scrobbler scrobbler = new Scrobbler();
			date = new Date();
			sec = date.getTime() / 1000;
			scrobbler.scrobbleTrack(args[1], args[2], args[3], 210, (int) sec);
		} catch (Exception e) {
			System.out.println("Caught exception: " + e.getMessage());
		}
	}

	public void scrobbleTrack(String artist, String title, String album, int length, int curtime) throws LastFMScrobblerException {
		String uartist = null, utitle = null, ualbum = null;
		try { 
			uartist = URLEncoder.encode(artist, "UTF-8");
			utitle = URLEncoder.encode(title, "UTF-8");
			ualbum = URLEncoder.encode(album, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			throw LastFMScrobblerException("Cannot convert to UTF-8: " + e.getMessage());
		}
		return;
	}

	private String authKey(String pass) throws LastFMScrobblerException { 
		long sec;
		Date date;
		MessageDigest md;
		BigInteger i;		
		String passhash;		
		try { 
			date = new Date();
			sec = date.getTime() / 1000;
			md = MessageDigest.getInstance("MD5");
			md.update(pass.getBytes("iso-8859-1"), 0, pass.length());
			i = new BigInteger(1,md.digest());
			passhash = String.format("%1$032x", i) + String.valueOf(sec);
			md = MessageDigest.getInstance("MD5");
			md.update(passhash.getBytes("iso-8859-1"), 0, passhash.length());
			i = new BigInteger(1,md.digest());
			return String.format("%1$032x", i);
		} catch (Exception e) {
			throw LastFMScrobblerException("Cannot generate authkey!");
		}
	}

	private String getSessionKey() {
		String authKey = authKey(password);
		/* XXX: Code goes here */
		return "";
	}

	private String doAskterisks(String pass) { 
		String ob = "";
		for(int i = 0; i < pass.length(); i++) { 
			ob += "*";
		}
		return ob;
	}

	private void readConfig(String file) {
		BufferedReader in = null;
		try {
			in = new BufferedReader(new FileReader(file));
			username = in.readLine();
			password = in.readLine();
		} catch(java.io.IOException e) {
			System.err.println("Caught java.io.IOException: " + e.getMessage());
		} finally {
			if(in != null) {
				try {
					in.close();
				} catch(java.io.IOException e) {
					System.err.println("Caught java.io.IOException: " + e.getMessage());
				} 	
			}
		}
	}
}


