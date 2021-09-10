#include<stdint.h>
#include<list>
#include<string>
#include<vector>
#include<iostream>
#include<bitset>
#include<map>
#include<fstream>
#include<exception>

#include<Windows.h>

#include"instructions.h"

using namespace std;

class Memory {
public:

	bitset<0x8000> ROM;	//0x0000-0x7fff
	bitset<0x4000> RAM;	//0x8000-0xbfff
	bitset<0x17> VRAM;	//0xc000-0xc016 0xc017-0xc01f:reserved 0xc020-0xdfff:mirror 
	bitset<0x5> ARAM;	//0xe000-0xe004 0xe005-0xe007:reserved 0xe008-0xefff:mirror
	bitset<0x5> ControllerInput0;	//0xf000-0xf004 0xf005-0xf007:reserved
	bitset<0x5> ControllerInput1;	//0xf008-0xf00c 0xf00d-0xf00f:reserved 0xf010-0xf7ff:mirror
	//0xf800-0xffff:reserved

	Memory() {

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

	uint8_t read6(uint16_t address) {
		uint8_t out = 0;
		for (uint8_t i = 0; i < 6; i++)
		{
			out |= read(address + i) << i;
		}
		return out;
	}

	uint16_t read16(uint16_t address) {
		uint16_t out = 0;
		for (uint8_t i = 0; i < 16; i++)
		{
			out |= read(address + i) << i;
		}
		return out;
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

	void write(uint16_t address, vector<bool> value) {
		for (uint8_t i = 0; i < value.size(); i++)
		{
			write(address + i, value[i]);
		}
	}

	void write(uint16_t address, uint16_t value) {
		for (uint8_t i = 0; i < 16; i++)
		{
			write(address + i, (bool)((value >> i) & 1));
		}
	}
};

enum class TokenError {
	OK,
	UnexpectedEndOfFile,
	IllegalOperand
};

class Token {
public:
	wstring token = L"";
	wstring filename = L"";
	TokenError errorType = TokenError::OK;
	basic_string<wchar_t>::size_type line = 0;
	basic_string<wchar_t>::size_type digit = 0;
};

class ParserError : public runtime_error {
public:
	Token token;
	ParserError(string message, Token _token) :runtime_error(message) {
		token = _token;
	}
};

class BBBBrainDumbed {
public:
	uint16_t Z = 0, X = 0, Y = 0, A = 0, B = 0, D = 0, E = 0, P = 0, V = 0, T = 0;
	uint8_t I = 0, J = 0, inst = 0, stage = 0;
	bool C = false, M = false, IRQ = false;
	Memory memory;

	static list<Token>* Tokenizer(wstring input, wstring filename) {
		list<Token>* output = new list<Token>();
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
			if (input[i] == L':' || input[i] == L',' || input[i] == L'(' || input[i] == L')' || input[i] == L'+' || input[i] == L'-' || input[i] == L'*' || input[i] == L'/' || input[i] == L'%' || input[i] == L'~')	//label and operands
			{
				tmp.filename = filename;
				tmp.token.push_back(input[i]);
				i++;
				digit++;
				output->push_back(tmp);
				continue;
			}
			if (input[i] == L'<')
			{
				if ((i + 1) < input.length() && (input[i + 1] == L'<' || input[i + 1] == L'='))
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'>')
			{
				if ((i + 2) < input.length() && input[i + 1] == L'>' && input[i + 2] == L'>')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					tmp.token.push_back(input[i + 2]);
					i += 3;
					digit += 3;
					output->push_back(tmp);
					continue;
				}
				else if ((i + 1) < input.length() && (input[i + 1] == L'>' || input[i + 1] == L'='))
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'|')
			{
				if ((i + 1) < input.length() && input[i + 1] == L'|')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'&')
			{
				if ((i + 1) < input.length() && input[i + 1] == L'&')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'^')
			{
				if ((i + 1) < input.length() && input[i + 1] == L'^')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'!')
			{
				if ((i + 1) < input.length() && input[i + 1] == L'=')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
			}
			if (input[i] == L'=')
			{
				if ((i + 1) < input.length() && input[i + 1] == L'=')
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					tmp.token.push_back(input[i + 1]);
					i += 2;
					digit += 2;
					output->push_back(tmp);
					continue;
				}
				else
				{
					tmp.filename = filename;
					tmp.token.push_back(input[i]);
					i++;
					digit++;
					output->push_back(tmp);
					continue;
				}
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
			if (input[i] == L'\'')	//single quote
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
				output->push_back(tmp);
				continue;
			}
			if (input[i] == L'\"')	//double quote
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
				output->push_back(tmp);
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
				if ((i + 1) < input.length() && input[i + 1] == L'\n')
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
				while (i < input.length() && input[i] != L' ' && input[i] != L'\r' && input[i] != L'\n' && input[i] != L'\0' && input[i] != L'\t' && input[i] != L'+' && input[i] != L'-' && input[i] != L'*' && input[i] != L'/' && input[i] != L'%' && input[i] != L'|' && input[i] != L'&' && input[i] != L'^' && input[i] != L'~' && input[i] != L'<' && input[i] != L'>' && input[i] != L'!' && input[i] != L'=' && input[i] != L',' && input[i] != L'(' && input[i] != L')')	//not separator nor operand
				{
					tmp.token.push_back(input[i]);
					i++;
					digit++;
				}
				output->push_back(tmp);
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

	static bool hasNumber(wstring input, instructions insts) {
		/*
		followings has number: binary(start with 0b), quaternary(start with 0q), octal(start with 0o or 0), decimal(no prefix or start with 0d), hexadecimal(start with 0x), quoted text(surrounded by ' or "), identifier(enything else without end with :), label(enything else with end with :)
		followings does not have number: mnemonic, directive, operator
		*/
		auto i = insts.inst.find(input);
		if (i != insts.inst.end() && (i->second.itype == instructionType::mnemonic || i->second.itype == instructionType::directive || i->second.itype == instructionType::$operator))
		{
			return false;
		}
		return true;
	}

	static int64_t toNumber(wstring input, instructions insts) {
		auto i = insts.inst.find(input);
		if (i != insts.inst.end())
		{
			if (i->second.itype == instructionType::knownnumber || i->second.itype == instructionType::$operator)
			{
				return i->second.value;
			}
			else
			{
				throw runtime_error("unresolved value");
			}
		}
		else if (input[0] == L'0')
		{
			if (input[1] == L'b')
			{
				input.erase(0, 2);
				return stoll(input, nullptr, 2);
			}
			else if (input[1] == L'q')
			{
				input.erase(0, 2);
				return stoll(input, nullptr, 4);
			}
			else if (input[1] == L'o')
			{
				input.erase(0, 2);
				return stoll(input, nullptr, 8);
			}
			else if (input[1] == L'd')
			{
				input.erase(0, 2);
				return stoll(input, nullptr, 10);
			}
			else if (input[1] == L'x')
			{
				input.erase(0, 2);
				return stoll(input, nullptr, 16);
			}
			else
			{
				return stoll(input, nullptr, 0);
			}
		}
		else if (input[0] >= L'1' && input[0] <= L'9')
		{
			return stoll(input, nullptr, 10);
		}
		else if ((input[0] == L'\"' && input.back() == L'\"') || (input[0] == L'\'' && input.back() == L'\''))
		{
			input.erase(0, 1);
			input.erase(input.size() - 1, 1);
			input.resize(sizeof(wchar_t) * 4, 0);
			return (int64_t)*input.c_str();
		}
		else
		{
			throw runtime_error("not a number");
		}
		return 0;
	}

	static list<Token>::iterator peekToken(list<Token>* input, list<Token>::iterator* i) {
		auto j = ++(*i);
		--(*i);
		if (j == input->end())
		{
			return *i;
		}
		else
		{
			return j;
		}
	}

	static list<Token>::iterator getToken(list<Token>* input, list<Token>::iterator* i) {
		auto j = ++(*i);
		if (j == input->end())
		{
			return *i;
		}
		else
		{
			return j;
		}
	}

	static bool checkPrevToken(list<Token>* input, list<Token>::iterator* i, list<Token>::iterator begin, instructions insts) {
		bool output = false;
		if ((*i) == begin)
		{
			return true;
		}
		--(*i);
		auto j = insts.inst.find((*i)->token);
		if (j != insts.inst.end() && j->second.itype == instructionType::$operator)
		{
			output = true;
		}
		if ((*i)->token == L")")
		{
			output = true;
		}
		++(*i);
		return output;

	}

	static int64_t parse_terminal(list<Token>* input, list<Token>::iterator* i, list<Token>::iterator begin, instructions insts) {

		int64_t value = 0;
		if ((*i)->token == L"(")
		{
			getToken(input, i);
			value = parse(input, i, begin, insts, parse_terminal(input, i, begin, insts));
			getToken(input, i);
			if ((*i)->token != L")")
			{
				throw runtime_error("Right parenthesis missing");
			}
		}
		else if ((*i)->token == L"-" && checkPrevToken(input, i, begin, insts))	//unary minus if previous token does not exist or is operator or right parenthesis
		{
			getToken(input, i);
			value -= parse_terminal(input, i, begin, insts);
		}
		else if ((*i)->token == L"+" && checkPrevToken(input, i, begin, insts))
		{
			getToken(input, i);
			value += parse_terminal(input, i, begin, insts);
		}
		else if ((*i)->token == L"~" && checkPrevToken(input, i, begin, insts))
		{
			getToken(input, i);
			value = ~parse_terminal(input, i, begin, insts);
		}
		else if ((*i)->token == L"!" && checkPrevToken(input, i, begin, insts))
		{
			getToken(input, i);
			value = !parse_terminal(input, i, begin, insts);
		}
		else if (hasNumber((*i)->token, insts))
		{
			value = toNumber((*i)->token, insts);
		}
		else
		{
			
		}
		return value;
	}

	static int64_t parse(list<Token>* input, list<Token>::iterator* i, list<Token>::iterator begin, instructions insts, int64_t lhs, int64_t precedence = 0) {
		list<Token>::iterator j = peekToken(input, i);
		while ((j) != (*i) && (j->token != L")") && insts.inst.find((j)->token)->second.itype == instructionType::$operator && insts.inst.find((j)->token)->second.value >= precedence)
		{
			Token op = *j;
			getToken(input, i);
			getToken(input, i);
			int64_t rhs = parse_terminal(input, i, begin, insts);
			j = peekToken(input, i);
			while ((j != *i) && (j->token != L")") && ((insts.inst.find(op.token)->second.value < insts.inst.find((j)->token)->second.value) || (insts.inst.find((j)->token)->second.atype == associativity::right_associative && (insts.inst.find(op.token)->second.value == insts.inst.find((j)->token)->second.value))))
			{
				rhs = parse(input, i, begin, insts, rhs, insts.inst.find(op.token)->second.value + 1);
				j = peekToken(input, i);
			}
			if (op.token == L"+")
			{
				lhs += rhs;
			}
			else if (op.token == L"-")
			{
				lhs -= rhs;
			}
			else if (op.token == L"*")
			{
				lhs *= rhs;
			}
			else if (op.token == L"/")
			{
				lhs /= rhs;
			}
			else if (op.token == L"%")
			{
				lhs %= rhs;
			}
			else if (op.token == L"|")
			{
				lhs |= rhs;
			}
			else if (op.token == L"&")
			{
				lhs &= rhs;
			}
			else if (op.token == L"^")
			{
				lhs ^= rhs;
			}
			else if (op.token == L"<<")
			{
				lhs = lhs << rhs;
			}
			else if (op.token == L">>")
			{
				lhs = ((uint64_t)lhs) >> rhs;
			}
			else if (op.token == L">>>")
			{
				lhs = ((int64_t)lhs) >> rhs;
			}
			else if (op.token == L"||")
			{
				lhs = (lhs != 0) || (rhs != 0);
			}
			else if (op.token == L"&&")
			{
				lhs = (lhs != 0) && (rhs != 0);
			}
			else if (op.token == L"^^")
			{
				lhs = (lhs != 0) != (rhs != 0);
			}
			else if (op.token == L"<")
			{
				lhs = lhs < rhs;
			}
			else if (op.token == L">")
			{
				lhs = lhs > rhs;
			}
			else if (op.token == L"<=")
			{
				lhs = lhs <= rhs;
			}
			else if (op.token == L">=")
			{
				lhs = lhs >= rhs;
			}
			else if (op.token == L"!=")
			{
				lhs = lhs != rhs;;
			}
			else if (op.token == L"==")
			{
				lhs = lhs == rhs;
			}
		}
		return lhs;
	}

	static vector<bool> Parser(list<Token>* input) {
		vector<bool> output;
		map<list<Token>::iterator, wstring> labels;
		list<Token>::iterator i = input->begin();
		instructions insts;
		while (i != input->end())
		{
			for (basic_string<wchar_t>::size_type j = 0; j < (*i).token.length(); j++)
			{
				(*i).token[j] = towlower((*i).token[j]);
			}
			i++;
		}
		/*
		processing order: convert to binary (leave unresolved reference empty) -> resolve reference -> overwrite resolved reference -> end

		What to do if we meet like this:
			binclude "filename" filesize	;<- size need to know filesize
			filesize:	;<- filesize need to know size of binclude which defined by filesize (self reference)
		Dependency of label is all of previously appeared size-defining identifier
		*/
		i = input->begin();
		while (i != input->end())
		{
			auto j = insts.inst.find((*i).token);
			if (j == insts.inst.end())
			{
				if ((*i).token.back() == L':')	//label
				{
					wstring l = (*i).token;
					l.pop_back();
					auto k = insts.inst.find(l);
					if (k == insts.inst.end())
					{
						insts.inst.insert(make_pair(l, instruction(0, instructionType::unknownnumber, 0)));
					}
					else
					{
						throw runtime_error("multiple labels with same identifier found");
					}
				}
				else	//identifier
				{
					throw runtime_error("identifier must be come with mnemonic or directive");
				}
			}
			else
			{
				if (j->second.itype == instructionType::mnemonic)
				{
					for (size_t k = 0; k < j->second.opcode.size(); k++)
					{
						output.push_back(j->second.opcode.test(k));
					}
				}
				else if (j->second.itype == instructionType::directive)
				{
					if (j->first == L"binclude")	//format: binclude filename [filesize] [fileoffset]
					{
						i++;
						wstring filepath = (*i).token;
						size_t filesize = 0, fileoffset = 0;

						basic_ifstream<char> ifs;
						ifs.open(filepath, ios_base::binary | ios_base::in);
						if (ifs.fail())
						{
							throw runtime_error("failed to open file");
						}
						istreambuf_iterator<char> ifsbegin(ifs), ifsend;
						string finput(ifsbegin, ifsend);
						
						ifs.close();
					}
					else if (j->first == L"define")
					{
						auto k = ++i;
						if (i == input->end())
						{
							throw runtime_error("Unexpected end of file");
						}
						i++;
						int64_t l = parse(input, &i, i, insts, parse_terminal(input, &i, i, insts));
						if (insts.inst.find((*k).token) == insts.inst.end() || insts.inst.find((*k).token)->second.itype == instructionType::knownnumber || insts.inst.find((*k).token)->second.itype == instructionType::unknownnumber)
						{
							insts.inst.insert_or_assign((*k).token, instruction(0, instructionType::knownnumber, l));
						}
						else
						{
							throw ParserError("keyword cannot be used", *k);
						}
					}
				}
			}
			i++;
		}
		return output;
	}

	void Execute(size_t count) {
		size_t tick = 0;
		size_t inst_count = 0;
		while (tick < count)
		{
			switch (stage) {
			case 0:
				stage++;
				tick++;
				break;
			case 1:
				if (P <= (0xf000 - 6))
				{
					inst = memory.read6(P);
					P += 6;
					stage = 8;
					tick += 7;
				}
				else
				{
					P++;
					stage++;
					tick++;
				}
				break;
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				inst |= memory.read(P - 1) << (stage - 2);
				P++;
				stage++;
				tick++;
				break;
			case 7:
				inst |= memory.read(P - 1) << (stage - 2);
				stage++;
				tick++;
				break;
			case 8:
				inst_count++;
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
	wstring exepath, filepath;
	basic_ifstream<wchar_t> ifs;
	if (argc >= 2)
	{
		ifs.open(argv[1]);
		filepath = argv[1];
		if (ifs.fail())
		{
			return 2;
		}
		
	}
	else
	{
		return 1;
	}
	istreambuf_iterator<wchar_t> ifsbegin(ifs), ifsend;
	wstring finput(ifsbegin,ifsend);
	ifs.close();
	list<Token>* tokens = BBBBrainDumbed::Tokenizer(finput, filepath);
	BBBBrainDumbed::CheckTokenError(*tokens);
	vector<bool> ROM;
	try
	{
		ROM = BBBBrainDumbed::Parser(tokens);
	}
	catch (const ParserError& e)
	{
		wcout << L"Parser error at file:" << e.token.filename << L" line:" << e.token.line << L" digit:" << e.token.digit << endl << e.what() << endl;
		return 3;
	}
	catch (const runtime_error& e)
	{
		wcout << L"Parser error\n" << e.what() << endl;
		return 4;
	}
	BBBBrainDumbed b;
	b.memory.BakeRom(ROM);
	b.Z = 12345;
	b.X = 60000;
	b.Y = 10000;
	b.C = 1;
	b.A = 0x8000;
	b.memory.write(0x8000, (uint16_t)52149);
	LARGE_INTEGER qpc0, qpc1, qpf;
	QueryPerformanceFrequency(&qpf);
	QueryPerformanceCounter(&qpc0);
	for (size_t i = 0; i < 1; i++)
	{
		b.P = 0;
		b.Execute(1516881);
	}
	QueryPerformanceCounter(&qpc1);
	wcout << L"Z=" << b.Z << L" X=" << b.X << L" Y=" << b.Y << L" C=" << b.C << L" B=" << b.B << L" P=" << b.P << L" (0x8000)=" << b.memory.read16(0x8000) << endl;
	wcout << (double)(qpc1.QuadPart - qpc0.QuadPart) / qpf.QuadPart << endl;
	return 0;
}