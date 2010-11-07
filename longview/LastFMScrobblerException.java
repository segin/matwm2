package com.segin.LastFM;

class LastFMScrobblerException extends Exception
{
	static final long serialVersionUID = 0;

	public LastFMScrobblerException() {
		super();
	}

	public LastFMScrobblerException(String why) { 
		super(why);
	}
}
