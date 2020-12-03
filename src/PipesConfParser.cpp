#include "PipesConfParser.hpp"

#include <fcntl.h>
#include <unistd.h>

#include <iostream> //

//#define err(T) static_cast<int>(T)
//operator int()()

/* only ASCII, without \\
{
	"playback" : "/home/${user}/sound/playback",
	"capture": "~/sound/capture",
	"remote": false
}
*/

const char * userMark = "{user}";

bool isWhitespaceSymbol(char byte)
{
	if (byte == ' ' || byte == '\t' || byte == '\r' || byte == '\n')
		return true;
	return false;
}
bool isFilenameSymbol(char byte)
{
	if (byte >= ',' && byte <= '9')
		return true;
	if (byte >= 'A' && byte <= 'Z')
		return true;
	if (byte >= 'a' && byte <= 'z')
		return true;
	return false;
}

const char * ERROR_CODE_DESCRIPTIONS[] = {
	"no error",
	"syntax error",

	"can not open file",
	"file is incomplete",
	"" // for invalid error code
};

namespace StretchPlayer
{

PipesConfParser::PipesConfParser()
{
	//
}

int PipesConfParser::parse(const char *filepath)
{
	int fd = open(filepath, O_RDONLY);
	if (!fd)
		return static_cast<int>(Error::CanNotOpenFile);
	const int bufferSize = 1024;
	char buffer[bufferSize];
	initParse();
	while(int result = read(fd, buffer, bufferSize))
	{
		if (result < 0)
			return false;
		for (char i : buffer)
		{
			if (int err = static_cast<int>(parseTick(i)))
				return err;
		}
	}
	if (_state.s != State::S::Finished)
	{
		return static_cast<int>(Error::IncompleteFile);
	}
	puts("ok");//
	return 0;
}

const char * PipesConfParser::error(int errorCode)
{
	if (errorCode < 0 || errorCode >= static_cast<int>(Error::__Count))
	{
		return ERROR_CODE_DESCRIPTIONS[static_cast<int>(Error::__Count)];
	}
	return ERROR_CODE_DESCRIPTIONS[errorCode];
}

const PipesConf & PipesConfParser::result() const
{
	return _pipesConf;
}

void PipesConfParser::initParse()
{
	_state = State();
}

PipesConfParser::Error PipesConfParser::parseTick(char byte)
{
	if (_state.s == State::S::Init)
	{
		if (isWhitespaceSymbol((byte)))
			;
		else if (byte == '{')
			_state.s = State::S::IntoGlobalObject;
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::IntoGlobalObject)
	{
		if (isWhitespaceSymbol(byte))
			;
		else if (byte == '"'){
			_state.s = State::S::KeyStarting;
			_state.key.clear();
		}
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::KeyStarting)
	{
		if (byte == '"')
		{
			_state.s = State::S::KeyValueSeparator;
		}
		else if (isFilenameSymbol(byte))
		{
			_state.key.push_back(byte);
		}
		else
		{
			return Error::SystaxError;
		}
	}
	else if (_state.s == State::S::KeyValueSeparator)
	{
		if (isWhitespaceSymbol(byte))
			;
		else if (byte == ':')
		{
			if (_state.key == "playback")
				_state.s = State::S::ValuePlaybackStarting;
			else if (_state.key == "capture")
				_state.s = State::S::ValueCaptureStarting;
			else if (_state.key == "remote")
				_state.s = State::S::ValueRemote;
			else
				return Error::SystaxError;
		}
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::ValuePlaybackStarting)
	{
		if (byte == '"')
		{
			_state.s = State::S::ValuePlayback;
			_state.value.clear();
		}
		else if (isWhitespaceSymbol(byte))
			;
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::ValueCaptureStarting)
	{
		if (byte == '"')
		{
			_state.s = State::S::ValueCapture;
			_state.value.clear();
		}
		else if (isWhitespaceSymbol(byte))
			;
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::ValueRemote)
	{
		//
	}
	else if (_state.s == State::S::ValuePlayback)
	{
		if (byte == '"')
		{
			// boris here: save value
			//_pipesConf.playback = _state.key;
			_state.s = State::S::ValueFinished;
		}
		else if (isFilenameSymbol(byte))
		{
			_state.value.push_back(byte);
		}
		else if (byte == '~' && _state.value.size() == 0)
		{
			_state.s = State::S::ValuePlaybackTilda;
		}
		else if (byte == '$')
		{
			_state.s = State::S::ValuePlaybackDollar;
		}
	}
	else if (_state.s == State::S::ValueCapture)
	{
		//
	}
	else if (_state.s == State::S::ValuePlaybackTilda)
	{
		if (byte == '"')
		{
			// boris here: save value
			_state.s = State::S::ValueFinished;
		}
		else if (byte == '/')
		{
			_state.value.append("/home/boris/");
			_state.s = State::S::ValuePlayback;
		}
		else if (isFilenameSymbol(byte))
		{
			_state.value.push_back('~');
			_state.value.push_back(byte);
		}
		else if (byte == '$')
		{
			_state.value.push_back('~');
			_state.s = State::S::ValuePlaybackDollar;
		}
	}
	else if (_state.s == State::S::ValueCaptureTilda)
	{
		//
	}
	else if (_state.s == State::S::ValuePlaybackDollar)
	{
		if (true) // boris here: check for "{user}" using _s.counter. По успешном окончании выставляем состояние ValuePlayback
		{
			;
		}
		else
			return Error::SystaxError;
	}
	else if (_state.s == State::S::ValueCaptureDollar)
	{
		//
	}
	else if (_state.s == State::S::ValueFinished)
	{
		if (_state.key == "playback")
		{
			//_pipesConf.playback = _state.value;
			//printf("playback: %s", _state.value);//
			std::cout << "playback: " << _state.value << "\n"; //
		}
		else if (_state.key == "capture")
		{
			//_pipesConf.capture = _state.value;
			//printf("capture: %s", _state.value);//
			std::cout << "capture: " << _state.value << "\n"; //
		}
		else if (_state.key == "remote")
		{
			//
		}
		else
		{
			return Error::SystaxError;
		}
	}
	//
	return Error::NoError;
}

} // namespace StretchPlayer
