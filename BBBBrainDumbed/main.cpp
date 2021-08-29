#include<stdint.h>
#include<list>
#include<string>
#include<vector>
#include<iostream>
#include<bitset>

using namespace std;

class Memory {
public:

	vector<bool> ROM;	//0x0000-0x7fff
	vector<bool> RAM;	//0x8000-0xbfff
	vector<bool> VRAM;	//0xc000-0xc016 0xc017-0xc01f:reserved 0xc020-0xdfff:mirror 
	vector<bool> ARAM;	//0xe000-0xe004 0xe005-0xe007:reserved 0xe008-0xefff:mirror
	vector<bool> ControllerInput0;	//0xf000-0xf004 0xf005-0xf007:reserved
	vector<bool> ControllerInput1;	//0xf008-0xf00c 0xf00d-0xf00f:reserved 0xf010-0xf7ff:mirror
	//0xf800-0xffff:reserved

	Memory() {
		ROM.resize(0x8000, false);
		RAM.resize(0x4000, false);
		VRAM.resize(0x17, false);
		ARAM.resize(0x5, false);
		ControllerInput0.resize(0x5, false);
		ControllerInput1.resize(0x5, false);
	}

	void BakeRom(vector<bool> input) {
		if (input.size() > 0x8000)
		{
			throw out_of_range("Input is too large.");
		}
		for (size_t i = 0; i < input.size(); i++)
		{
			ROM[i] = input[i];
		}
	}

	uint16_t MapAddress(uint16_t input) {
		uint16_t output = input;
		if (input >= 0xc020 && input <= 0xdfff)
		{
			//1100 0000 0001 1111
			//1101 1111 1111 1111
			output = input & 0xc01f;
		}
		else if (input >= 0xe008 && input <= 0xefff)
		{
			//1110 0000 0000 0111
			//1110 1111 1111 1111
			output = input & 0xe007;
		}
		else if (input >= 0xf010 && input <= 0xf7ff)
		{
			//1111 0000 0000 1111
			//1111 0111 1111 1111
			output = input & 0xf00f;
		}
		return output;
	}

	bool read(uint16_t address) {
		uint16_t i = MapAddress(address);
		if (i <= 0x7fff)
		{
			return ROM[i];
		}
		else if (i <= 0xbfff)
		{
			return RAM[i - 0x8000];
		}
		else if (i <= 0xc016)
		{
			return VRAM[i - 0xc000];
		}
		else if (i <= 0xdfff)
		{
			return true;
		}
		else if (i <= 0xe004)
		{
			return ARAM[i - 0xe000];
		}
		else if (i <= 0xefff)
		{
			return true;
		}
		else if (i <= 0xf004)
		{
			return ControllerInput0[i - 0xf000];
		}
		else if (i <= 0xf007)
		{
			return true;
		}
		else if (i <= 0xf00c)
		{
			return ControllerInput1[i - 0xf008];
		}
		else
		{
			return true;
		}
	}

	void write(uint16_t address, bool value) {
		uint16_t i = MapAddress(address);
		if (i <= 0x7fff)
		{
			ROM[i] = value;
		}
		else if (i <= 0xbfff)
		{
			RAM[i - 0x8000] = value;
		}
		else if (i <= 0xc016)
		{
			VRAM[i - 0xc000] = value;
		}
		else if (i <= 0xdfff)
		{
			return;
		}
		else if (i <= 0xe004)
		{
			ARAM[i - 0xe000] = value;
		}
		else if (i <= 0xefff)
		{
			return;
		}
		else if (i <= 0xf004)
		{
			ControllerInput0[i - 0xf000] = value;
		}
		else if (i <= 0xf007)
		{
			return;
		}
		else if (i <= 0xf00c)
		{
			ControllerInput1[i - 0xf008] = value;
		}
		else
		{
			return;
		}
	}
};

enum class TokenError {
	OK = 0,
	UnexpectedEndOfFile = 1
};

class Token {
public:
	wstring token = L"";
	wstring filename = L"";
	TokenError errorType = TokenError::OK;
	basic_string<wchar_t>::size_type line = 0;
	basic_string<wchar_t>::size_type digit = 0;
};

class BBBBrainDumbed {
public:
	uint16_t Z = 0, X = 0, Y = 0, A = 0, B = 0, D = 0, E = 0, P = 0, V = 0, T = 0;
	uint8_t I = 0, J = 0, inst = 0, stage = 0;
	bool C = false, M = false, IRQ = false;
	Memory memory;

	static list<Token> Tokenizer(wstring input, wstring filename) {
		list<Token> output;
		basic_string<wchar_t>::size_type i = 0;
		basic_string<wchar_t>::size_type line = 1;
		basic_string<wchar_t>::size_type digit = 1;
		while (true)
		{
			Token tmp;
			if (i >= input.length() || input[i] == L'\0')	//end of file
			{
				break;
			}
			if (input[i] == L':'/* || input[i] == L',' || input[i] == L'(' || input[i] == L')'*/ && false)	//label and others (unused)
			{
				tmp.filename = filename;
				tmp.token.push_back(input[i]);
				i++;
				digit++;
				output.push_back(tmp);
				continue;
			}
			if (input[i] == L';')	//comment
			{
				i++;
				digit++;
				while (input[i] != L'\n' && input[i] != L'\r')
				{
					i++;
					digit++;
				}
				continue;
			}
			
			if (input[i] == L'\'' && false)	//single quote (unused)
			{
				tmp.filename = filename;
				tmp.line = line;
				tmp.digit = digit;
				i++;
				digit++;
				while (true)
				{
					if (i >= input.length())
					{
						tmp.errorType = TokenError::UnexpectedEndOfFile;
						break;
					}
					if (input[i] == L'\\')
					{
						i++;
						digit++;
						if (i >= input.length())
						{
							tmp.errorType = TokenError::UnexpectedEndOfFile;
							break;
						}
						if (input[i] == L'a')
						{
							tmp.token.push_back(L'\a');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'b')
						{
							tmp.token.push_back(L'\b');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'f')
						{
							tmp.token.push_back(L'\f');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'n')
						{
							tmp.token.push_back(L'\n');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'r')
						{
							tmp.token.push_back(L'\r');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L't')
						{
							tmp.token.push_back(L'\t');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'v')
						{
							tmp.token.push_back(L'\v');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\\')
						{
							tmp.token.push_back(L'\\');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\'')
						{
							tmp.token.push_back(L'\'');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\"')
						{
							tmp.token.push_back(L'\"');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\?')
						{
							tmp.token.push_back(L'\?');
							i++;
							digit++;
							continue;
						}
						if (input[i] >= L'0' && input[i] <= L'7')
						{
							wstring tmpString = L"";
							while (input[i] >= L'0' && input[i] <= L'7')
							{
								tmpString.push_back(input[i]);
								i++;
								digit++;
							}
							basic_string<wchar_t>::size_type j = tmpString.length();
							while (j > 0)
							{
								basic_string<wchar_t>::size_type k = 0;
								uint64_t l = 0;
								while (k < 16 && j > 0)
								{
									l = l | (stoull(&tmpString[j - 1], nullptr, 8)) << (3 * k);
									//l = l | ((uint64_t)(tmpString[j - 1] - L'0') << 3 * k);
									tmpString.pop_back();
									j--;
									k++;
								}
								uint32_t m = 1;
								if (*(char*)& m)	//little-endian 0x1032547698ba----
								{
									for (basic_string<wchar_t>::size_type n = 0; (k * 3) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& l)[n]);
									}
								}
								else //big-endian 0x----ba9876543210
								{
									uint32_t o = 0;
									o |= l << 56;
									o |= (l & 0x00000000'0000ff00) << 40;
									o |= (l & 0x00000000'00ff0000) << 24;
									o |= (l & 0x00000000'ff000000) << 8;
									o |= (l & 0x000000ff'00000000) >> 8;
									o |= (l & 0x0000ff00'00000000) >> 24;
									o |= (l & 0x00ff0000'00000000) >> 40;
									o |= l >> 56;
									for (basic_string<wchar_t>::size_type n = 0; (k * 3) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& o)[n]);
									}
								}
							}
						}
						if (input[i] == L'x' || input[i] == L'X')
						{
							i++;
							digit++;
							wstring tmpString = L"";
							while ((input[i] >= L'0' && input[i] <= L'9') || (input[i] >= L'a' && input[i] <= L'f') || (input[i] >= L'A' && input[i] <= L'F'))
							{
								tmpString.push_back(input[i]);
								i++;
								digit++;
							}
							basic_string<wchar_t>::size_type j = tmpString.length();
							while (j > 0)
							{
								basic_string<wchar_t>::size_type k = 0;
								uint64_t l = 0;
								while (k < 16 && j > 0)
								{
									//l = l | (_wtoll(&tmpString[j - 1]) << (4 * k));
									l = l | (stoull(&tmpString[j - 1], nullptr, 16)) << (4 * k);
									//l = l | ((uint64_t)(tmpString[j - 1] - L'0') << 3 * k);
									tmpString.pop_back();
									j--;
									k++;
								}
								uint32_t m = 1;
								if (*(char*)& m)	//little-endian 0x1032547698ba----
								{
									for (basic_string<wchar_t>::size_type n = 0; (k * 4) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& l)[n]);
									}
								}
								else //big-endian 0x----ba9876543210
								{
									uint32_t o = 0;
									o |= l << 56;
									o |= (l & 0x00000000'0000ff00) << 40;
									o |= (l & 0x00000000'00ff0000) << 24;
									o |= (l & 0x00000000'ff000000) << 8;
									o |= (l & 0x000000ff'00000000) >> 8;
									o |= (l & 0x0000ff00'00000000) >> 24;
									o |= (l & 0x00ff0000'00000000) >> 40;
									o |= l >> 56;
									for (basic_string<wchar_t>::size_type n = 0; (k * 4) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& o)[n]);
									}
								}
							}
						}
					}
					if (input[i] == L'\'')
					{
						i++;
						digit++;
						break;
					}
					tmp.token.push_back(input[i]);
					i++;
					digit++;
				}
				output.push_back(tmp);
				continue;
			}
			if (input[i] == L'\"' && false)	//double quote (unused)
			{
				tmp.filename = filename;
				tmp.line = line;
				tmp.digit = digit;
				i++;
				digit++;
				while (true)
				{
					if (i >= input.length())
					{
						tmp.errorType = TokenError::UnexpectedEndOfFile;
						break;
					}
					if (input[i] == L'\\')
					{
						i++;
						digit++;
						if (i >= input.length())
						{
							tmp.errorType = TokenError::UnexpectedEndOfFile;
							break;
						}
						if (input[i] == L'a')
						{
							tmp.token.push_back(L'\a');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'b')
						{
							tmp.token.push_back(L'\b');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'f')
						{
							tmp.token.push_back(L'\f');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'n')
						{
							tmp.token.push_back(L'\n');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'r')
						{
							tmp.token.push_back(L'\r');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L't')
						{
							tmp.token.push_back(L'\t');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'v')
						{
							tmp.token.push_back(L'\v');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\\')
						{
							tmp.token.push_back(L'\\');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\'')
						{
							tmp.token.push_back(L'\'');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\"')
						{
							tmp.token.push_back(L'\"');
							i++;
							digit++;
							continue;
						}
						if (input[i] == L'\?')
						{
							tmp.token.push_back(L'\?');
							i++;
							digit++;
							continue;
						}
						if (input[i] >= L'0' && input[i] <= L'7')
						{
							wstring tmpString = L"";
							while (input[i] >= L'0' && input[i] <= L'7')
							{
								tmpString.push_back(input[i]);
								i++;
								digit++;
							}
							basic_string<wchar_t>::size_type j = tmpString.length();
							while (j > 0)
							{
								basic_string<wchar_t>::size_type k = 0;
								uint64_t l = 0;
								while (k < 16 && j > 0)
								{
									l = l | (stoull(&tmpString[j - 1], nullptr, 8)) << (3 * k);
									//l = l | ((uint64_t)(tmpString[j - 1] - L'0') << 3 * k);
									tmpString.pop_back();
									j--;
									k++;
								}
								uint32_t m = 1;
								if (*(char*)& m)	//little-endian 0x1032547698ba----
								{
									for (basic_string<wchar_t>::size_type n = 0; (k * 3) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& l)[n]);
									}
								}
								else //big-endian 0x----ba9876543210
								{
									uint32_t o = 0;
									o |= l << 56;
									o |= (l & 0x00000000'0000ff00) << 40;
									o |= (l & 0x00000000'00ff0000) << 24;
									o |= (l & 0x00000000'ff000000) << 8;
									o |= (l & 0x000000ff'00000000) >> 8;
									o |= (l & 0x0000ff00'00000000) >> 24;
									o |= (l & 0x00ff0000'00000000) >> 40;
									o |= l >> 56;
									for (basic_string<wchar_t>::size_type n = 0; (k * 3) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& o)[n]);
									}
								}
							}
						}
						if (input[i] == L'x' || input[i] == L'X')
						{
							i++;
							digit++;
							wstring tmpString = L"";
							while ((input[i] >= L'0' && input[i] <= L'9') || (input[i] >= L'a' && input[i] <= L'f') || (input[i] >= L'A' && input[i] <= L'F'))
							{
								tmpString.push_back(input[i]);
								i++;
								digit++;
							}
							basic_string<wchar_t>::size_type j = tmpString.length();
							while (j > 0)
							{
								basic_string<wchar_t>::size_type k = 0;
								uint64_t l = 0;
								while (k < 16 && j > 0)
								{
									//l = l | (_wtoll(&tmpString[j - 1]) << (4 * k));
									l = l | (stoull(&tmpString[j - 1], nullptr, 16)) << (4 * k);
									//l = l | ((uint64_t)(tmpString[j - 1] - L'0') << 3 * k);
									tmpString.pop_back();
									j--;
									k++;
								}
								uint32_t m = 1;
								if (*(char*)& m)	//little-endian 0x1032547698ba----
								{
									for (basic_string<wchar_t>::size_type n = 0; (k * 4) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& l)[n]);
									}
								}
								else //big-endian 0x----ba9876543210
								{
									uint32_t o = 0;
									o |= l << 56;
									o |= (l & 0x00000000'0000ff00) << 40;
									o |= (l & 0x00000000'00ff0000) << 24;
									o |= (l & 0x00000000'ff000000) << 8;
									o |= (l & 0x000000ff'00000000) >> 8;
									o |= (l & 0x0000ff00'00000000) >> 24;
									o |= (l & 0x00ff0000'00000000) >> 40;
									o |= l >> 56;
									for (basic_string<wchar_t>::size_type n = 0; (k * 4) > (sizeof(wchar_t) * 8 * n); n++)
									{
										tmp.token.push_back(((wchar_t*)& o)[n]);
									}
								}
							}
						}
					}
					if (input[i] == L'\"')
					{
						i++;
						digit++;
						break;
					}
					tmp.token.push_back(input[i]);
					i++;
					digit++;
				}
				output.push_back(tmp);
				continue;
			}
			if (input[i] == L' ' || input[i] == L'\t')	//space and tab
			{
				i++;
				digit++;
				continue;
			}
			if (input[i] == L'\r')	//return
			{
				if ((i + 1) <= input.length() && input[i + 1] == L'\n')
				{
					i += 2;
					digit = 1;
					line++;
					continue;
				}
				else
				{
					i++;
					digit = 1;
					line++;
					continue;
				}
			}
			if (input[i] == L'\n')	//linefeed
			{
				i++;
				digit = 1;
				line++;
				continue;
			}
			if (true)	//others
			{
				tmp.filename = filename;
				tmp.line = line;
				tmp.digit = digit;
				while (i < input.length() && input[i] != L' ' && input[i] != L'\r' && input[i] != L'\n' && input[i] != L'\0' && input[i] != L'\t')	//not separator
				{
					tmp.token.push_back(input[i]);
					i++;
					digit++;
				}
				output.push_back(tmp);
				continue;
			}
		}
		return output;
	}

	static int CheckTokenError(list<Token> input) {
		int error = 0;
		list<Token>::iterator i = input.begin();
		while (i != input.end())
		{
			if ((*i).errorType != TokenError::OK)
			{
				wstring errorType = L"";
				switch ((*i).errorType)
				{
				case TokenError::UnexpectedEndOfFile:
					errorType = L"UnexpectedEndOfFile";
					break;
				default:
					break;
				}
				wcout << errorType << L" in " << (*i).filename << L" at line " << (*i).line << L" digit " << (*i).digit << endl;
				error++;
			}
			i++;
		}
		return error;
	}

	static vector<bool> Parser(list<Token> input) {
		vector<bool> output;
		list<Token>::iterator i = input.begin();
		while (i != input.end())
		{
			for (basic_string<wchar_t>::size_type j = 0; j < (*i).token.length(); j++)
			{
				(*i).token[j] = towlower((*i).token[j]);
			}
			i++;
		}
		i = input.begin();
		while (i != input.end())
		{
			if ((*i).token == L"nop" || (*i).token == L"mtn")
			{
				output.insert(output.end(), { false, false, false, false, false, false });
			}
			else if ((*i).token == L"mtx")
			{
				output.insert(output.end(), { true, false, false, false, false, false });
			}
			else if ((*i).token == L"mty")
			{
				output.insert(output.end(), { false, true, false, false, false, false });
			}
			else if ((*i).token == L"mta")
			{
				output.insert(output.end(), { true, true, false, false, false, false });
			}
			else if ((*i).token == L"mtb")
			{
				output.insert(output.end(), { false, false, true, false, false, false });
			}
			else if ((*i).token == L"mtd")
			{
				output.insert(output.end(), { true, false, true, false, false, false });
			}
			else if ((*i).token == L"mte")
			{
				output.insert(output.end(), { false, true, true, false, false, false });
			}
			else if ((*i).token == L"mtp")
			{
				output.insert(output.end(), { true, true, true, false, false, false });
			}
			else if ((*i).token == L"mfn")
			{
				output.insert(output.end(), { false, false, false, true, false, false });
			}
			else if ((*i).token == L"mfx")
			{
				output.insert(output.end(), { true, false, false, true, false, false });
			}
			else if ((*i).token == L"mfy")
			{
				output.insert(output.end(), { false, true, false, true, false, false });
			}
			else if ((*i).token == L"mfa")
			{
				output.insert(output.end(), { true, true, false, true, false, false });
			}
			else if ((*i).token == L"mfb")
			{
				output.insert(output.end(), { false, false, true, true, false, false });
			}
			else if ((*i).token == L"mfd")
			{
				output.insert(output.end(), { true, false, true, true, false, false });
			}
			else if ((*i).token == L"mfe")
			{
				output.insert(output.end(), { false, true, true, true, false, false });
			}
			else if ((*i).token == L"mfp")
			{
				output.insert(output.end(), { true, true, true, true, false, false });
			}
			else if ((*i).token == L"bse")
			{
				output.insert(output.end(), { false, false, false, false, true, false });
			}
			else if ((*i).token == L"bnt")
			{
				output.insert(output.end(), { true, false, false, false, true, false });
			}
			else if ((*i).token == L"bor")
			{
				output.insert(output.end(), { false, true, false, false, true, false });
			}
			else if ((*i).token == L"ban")
			{
				output.insert(output.end(), { true, true, false, false, true, false });
			}
			else if ((*i).token == L"bxo")
			{
				output.insert(output.end(), { false, false, true, false, true, false });
			}
			else if ((*i).token == L"not")
			{
				output.insert(output.end(), { true, false, true, false, true, false });
			}
			else if ((*i).token == L"shl")
			{
				output.insert(output.end(), { false, true, true, false, true, false });
			}
			else if ((*i).token == L"shr")
			{
				output.insert(output.end(), { true, true, true, false, true, false });
			}
			else if ((*i).token == L"asr")
			{
				output.insert(output.end(), { false, false, false, true, true, false });
			}
			else if ((*i).token == L"ror")
			{
				output.insert(output.end(), { true, false, false, true, true, false });
			}
			else if ((*i).token == L"ad1")
			{
				output.insert(output.end(), { false, true, false, true, true, false });
			}
			else if ((*i).token == L"ad4")
			{
				output.insert(output.end(), { true, true, false, true, true, false });
			}
			else if ((*i).token == L"ldr")
			{
				output.insert(output.end(), { false, false, true, true, true, false });
			}
			else if ((*i).token == L"str")
			{
				output.insert(output.end(), { true, false, true, true, true, false });
			}
			else if ((*i).token == L"mtj")
			{
				output.insert(output.end(), { false, true, true, true, true, false });
			}
			else if ((*i).token == L"mfj")
			{
				output.insert(output.end(), { true, true, true, true, true, false });
			}
			else if ((*i).token == L"ld0")
			{
			output.insert(output.end(), { false, false, false, false, false, true });
			}
			else if ((*i).token == L"ld1")
			{
			output.insert(output.end(), { true, false, false, false, false, true });
			}
			else if ((*i).token == L"ld2")
			{
			output.insert(output.end(), { false, true, false, false, false, true });
			}
			else if ((*i).token == L"ld3")
			{
			output.insert(output.end(), { true, true, false, false, false, true });
			}
			else if ((*i).token == L"ld4")
			{
			output.insert(output.end(), { false, false, true, false, false, true });
			}
			else if ((*i).token == L"ld5")
			{
			output.insert(output.end(), { true, false, true, false, false, true });
			}
			else if ((*i).token == L"ld6")
			{
			output.insert(output.end(), { false, true, true, false, false, true });
			}
			else if ((*i).token == L"ld7")
			{
			output.insert(output.end(), { true, true, true, false, false, true });
			}
			else if ((*i).token == L"ld8")
			{
			output.insert(output.end(), { false, false, false, true, false, true });
			}
			else if ((*i).token == L"ld9")
			{
			output.insert(output.end(), { true, false, false, true, false, true });
			}
			else if ((*i).token == L"lda")
			{
			output.insert(output.end(), { false, true, false, true, false, true });
			}
			else if ((*i).token == L"ldb")
			{
			output.insert(output.end(), { true, true, false, true, false, true });
			}
			else if ((*i).token == L"ldc")
			{
			output.insert(output.end(), { false, false, true, true, false, true });
			}
			else if ((*i).token == L"ldd")
			{
			output.insert(output.end(), { true, false, true, true, false, true });
			}
			else if ((*i).token == L"lde")
			{
			output.insert(output.end(), { false, true, true, true, false, true });
			}
			else if ((*i).token == L"ldf")
			{
			output.insert(output.end(), { true, true, true, true, false, true });
			}
			else if ((*i).token == L"clc")
			{
			output.insert(output.end(), { false, false, false, false, true, true });
			}
			else if ((*i).token == L"sec")
			{
			output.insert(output.end(), { true, false, false, false, true, true });
			}
			else if ((*i).token == L"clm")
			{
			output.insert(output.end(), { false, true, false, false, true, true });
			}
			else if ((*i).token == L"sem")
			{
			output.insert(output.end(), { true, true, false, false, true, true });
			}
			else if ((*i).token == L"cli")
			{
			output.insert(output.end(), { false, false, true, false, true, true });
			}
			else if ((*i).token == L"clj")
			{
			output.insert(output.end(), { true, false, true, false, true, true });
			}
			else if ((*i).token == L"bzz")
			{
			output.insert(output.end(), { false, true, true, false, true, true });
			}
			else if ((*i).token == L"bcc")
			{
			output.insert(output.end(), { true, true, true, false, true, true });
			}
			else if ((*i).token == L"mtv")
			{
			output.insert(output.end(), { false, false, false, true, true, true });
			}
			else if ((*i).token == L"mfv")
			{
			output.insert(output.end(), { true, false, false, true, true, true });
			}
			else if ((*i).token == L"mti")
			{
			output.insert(output.end(), { false, true, false, true, true, true });
			}
			else if ((*i).token == L"mfi")
			{
			output.insert(output.end(), { true, true, false, true, true, true });
			}
			else if ((*i).token == L"mtc")
			{
			output.insert(output.end(), { false, false, true, true, true, true });
			}
			else if ((*i).token == L"mfc")
			{
			output.insert(output.end(), { true, false, true, true, true, true });
			}
			else if ((*i).token == L"mtm")
			{
			output.insert(output.end(), { false, true, true, true, true, true });
			}
			else if ((*i).token == L"mfm")
			{
			output.insert(output.end(), { true, true, true, true, true, true });
			}
			i++;
		}
		return output;
	}

	void Execute(size_t count) {
		size_t tick = 0;
		while (tick < count)
		{
			switch (stage) {
			case 0:
				stage++;
				tick++;
				break;
			case 1:
				P++;
				stage++;
				tick++;
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				inst = (uint8_t)bitset<6>(inst).set(stage - 2, memory.read(P - 1)).to_ulong();
				P++;
				stage++;
				tick++;
				break;
			case 7:
				inst = (uint8_t)bitset<6>(inst).set(stage - 2, memory.read(P - 1)).to_ulong();
				stage++;
				tick++;
				break;
			case 8:
				switch (inst)
				{
				case 0:	//nop,mtn
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 1:	//mtx
					X = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 2:	//mty
					Y = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 3:	//mta
					A = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 4:	//mtb
					B = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 5:	//mtd
					D = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 6:	//mte
					E = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 7:	//mtp
					P = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 8:	//mfn
					Z = 0;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 9:	//mfx
					Z = X;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 10:	//mfy
					Z = Y;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 11:	//mfa
					Z = A;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 12:	//mfb
					Z = B;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 13:	//mfd
					Z = D;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 14:	//mfe
					Z = E;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 15:	//mfp
					Z = P;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 16:	//bse
					Z = (uint16_t)bitset<16>(Z).set(I, bitset<16>(X).test(I)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 17:	//bnt
					Z = (uint16_t)bitset<16>(Z).set(I, !bitset<16>(X).test(I)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 18:	//bor
					Z = (uint16_t)bitset<16>(Z).set(I, bitset<16>(X).test(I) | bitset<16>(Y).test(I)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 19:	//ban
					Z = (uint16_t)bitset<16>(Z).set(I, bitset<16>(X).test(I) & bitset<16>(Y).test(I)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 20:	//bxo
					Z = (uint16_t)bitset<16>(Z).set(I, bitset<16>(X).test(I) ^ bitset<16>(Y).test(I)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 21:	//not
					Z = ~Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 22:	//shl
					T = (X << 1) | (X >> 15);
					Z = (uint16_t)bitset<16>(T).reset(I).to_ulong();
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 23:	//shr
					T = (X << 15) | (X >> 1);
					Z = (uint16_t)bitset<16>(T).reset((I - 1) & 0xf).to_ulong();
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 24:	//asr
					T = (X << 15) | (X >> 1);
					Z = (uint16_t)bitset<16>(T).set((I - 1) & 0xf, bitset<16>(X).test((I - 1) & 0xf)).to_ulong();
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 25:	//ror
					Z = (X << 15) | (X >> 1);
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 26:	//ad1
					T = (uint16_t)bitset<16>(X).test(I) + (uint16_t)bitset<16>(Y).test(I) + (uint16_t)C;
					C = (T & 3) >> 1;
					Z = (uint16_t)bitset<16>(Z).set(I, (bool)(T & 1)).to_ulong();
					I = (I + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 27:	//ad4
					T = ((X << (16 - I) | X >> I) & 0xf) + ((Y << (16 - I) | Y >> I) & 0xf) + (uint16_t)C;
					C = (T & 0x10) >> 4;
					T = T & 0xf;
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | T;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 28:	//ldr
					stage++;
					tick++;
					break;
				case 29:	//str
					stage++;
					tick++;
					break;
				case 30:	//mtj
					J = (uint8_t)(((Z << (16 - I)) | (X >> I)) & 0xf);
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 31:	//mfj
					Z = J;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 32:	//ld0
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x0;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 33:	//ld1
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x1;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 34:	//ld2
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x2;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 35:	//ld3
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x3;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 36:	//ld4
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x4;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 37:	//ld5
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x5;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 38:	//ld6
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x6;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 39:	//ld7
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x7;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 40:	//ld8
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x8;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 41:	//ld9
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0x9;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 42:	//lda
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xa;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 43:	//ldb
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xb;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 44:	//ldc
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xc;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 45:	//ldd
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xd;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 46:	//lde
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xe;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 47:	//ldf
					T = ((Z << (16 - I) | Z >> I) & 0xfff0) | 0xf;
					Z = T << (I) | T >> (16 - I);
					I = (I + 4) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 48:	//clc
					C = false;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 49:	//sec
					C = true;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 50:	//clm
					M = false;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 51:	//sem
					M = true;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 52:	//cli
					I = 0;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 53:	//clj
					J = 0;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 54:	//bzz
					if (Z == 0)
					{
						P = A;
					}
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 55:	//bcc
					if (C == false)
					{
						P = A;
					}
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 56:	//mtv
					V = Z;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 57:	//mfv
					Z = V;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 58:	//mti
					I = (uint8_t)(((Z << (16 - I)) | (X >> I)) & 0xf);
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 59:	//mfi
					Z = I;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 60:	//mtc
					C = bitset<16>(Z).test(I);
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 61:	//mfc
					Z = C;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 62:	//mtm
					M = bitset<16>(Z).test(I);
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 63:	//mfm
					Z = M;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				default:
					_ASSERT_EXPR(false, L"Unreachable condition reached.");
					break;
				}
				break;
			case 9:
				switch (inst)
				{
				case 28:
				case 29:
					stage++;
					tick++;
					break;
				default:
					wcout << L"inst=" << inst << endl;
					_ASSERT_EXPR(false,L"Unreachable condition reached.");
					break;
				}
				break;
			case 10:
				switch (inst)
				{
				case 28:	//ldr
					Z = (uint16_t)bitset<16>(Z).set(J, memory.read(A)).to_ulong();
					J = (J + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				case 29:	//str
					memory.write(A, bitset<16>(Z).test(J));
					J = (J + 1) & 0xf;
					checkIRQ();
					stage = 1;
					tick++;
					break;
				default:
					_ASSERT_EXPR(false, L"Unreachable condition reached.");
					break;
				}
				break;
			default:
				_ASSERT_EXPR(false, L"Unreachable condition reached.");
				break;
			}
		}
	}

	void checkIRQ() {
		if (!M && IRQ)
		{
			T = P;
			P = V;
			V = T;
		}
	}
};

int wmain(int argc, wchar_t* argv[], wchar_t* envp[]) {
	list<Token> tokens = BBBBrainDumbed::Tokenizer(LR"(cli clj mfn mty
;loop:
mfb ldr mtb mfa
mtx sec
ad4 ad4 ad4 ad4
mtx 
ld6 ld9 ld0 ld0
mta mfj
bzz mfx mta
ld8 ld1 ld0 ld0
mtp
;end:
)", L"");
	BBBBrainDumbed::CheckTokenError(tokens);
	vector<bool> ROM = BBBBrainDumbed::Parser(tokens);
	BBBBrainDumbed b;
	b.memory.BakeRom(ROM);
	b.X = 60000;
	b.Y = 10000;
	b.C = 1;
	b.Execute(0x100);
	wcout << L"Z=" << b.Z << L" X=" << b.X << L" Y=" << b.Y << L" C=" << b.C << endl;
	return 0;
}