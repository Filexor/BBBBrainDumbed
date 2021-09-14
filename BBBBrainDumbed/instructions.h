#include<string>
#include<bitset>
#include<map>

using namespace std;

enum class instructionType
{
	mnemonic,
	directive,
	$operator,
	unknownnumber,
	knownnumber
};

enum class associativity
{
	left_associative,
	right_associative,
};

class instruction {
public:
	bitset<6> opcode;
	instructionType itype;
	int64_t value;
	associativity atype;

	instruction(unsigned long long _opcode, instructionType _itype, int64_t _value) {
		opcode = _opcode;
		itype = _itype;
		value = _value;
		atype = associativity::left_associative;
	}
	instruction(unsigned long long _opcode, instructionType _itype, int64_t _value, associativity _atype) {
		opcode = _opcode;
		itype = _itype;
		value = _value;
		atype = _atype;
	}
};

class instructions {
public:
	map<wstring, instruction> inst;

	instructions() {
		inst.insert(make_pair(L"nop", instruction(0, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtn", instruction(0, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtx", instruction(1, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mty", instruction(2, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mta", instruction(3, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtb", instruction(4, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtd", instruction(5, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mte", instruction(6, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtp", instruction(7, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfn", instruction(8, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfx", instruction(9, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfy", instruction(10, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfa", instruction(11, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfb", instruction(12, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfd", instruction(13, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfe", instruction(14, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfp", instruction(15, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bse", instruction(16, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bnt", instruction(17, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bor", instruction(18, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ban", instruction(19, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bxo", instruction(20, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"not", instruction(21, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"shl", instruction(22, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"shr", instruction(23, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"asr", instruction(24, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ror", instruction(25, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ad1", instruction(26, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ad4", instruction(27, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ldr", instruction(28, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"str", instruction(29, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtj", instruction(30, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfj", instruction(31, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld0", instruction(32, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld1", instruction(33, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld2", instruction(34, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld3", instruction(35, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld4", instruction(36, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld5", instruction(37, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld6", instruction(38, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld7", instruction(39, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld8", instruction(40, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ld9", instruction(41, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"lda", instruction(42, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ldb", instruction(43, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ldc", instruction(44, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ldd", instruction(45, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"lde", instruction(46, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"ldf", instruction(47, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"clc", instruction(48, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"sec", instruction(49, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"clm", instruction(50, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"sem", instruction(51, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"cli", instruction(52, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"clj", instruction(53, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bzz", instruction(54, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"bcc", instruction(55, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtv", instruction(56, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfv", instruction(57, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mti", instruction(58, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfi", instruction(59, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtc", instruction(60, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfc", instruction(61, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mtm", instruction(62, instructionType::mnemonic, 0)));
		inst.insert(make_pair(L"mfm", instruction(63, instructionType::mnemonic, 0)));

		inst.insert(make_pair(L"binclude", instruction(0, instructionType::directive, 0)));
		inst.insert(make_pair(L"=", instruction(0, instructionType::directive, 0)));	//define
		inst.insert(make_pair(L"define", instruction(0, instructionType::directive, 0)));
		inst.insert(make_pair(L"equ", instruction(0, instructionType::directive, 0)));
		inst.insert(make_pair(L"ldi", instruction(0, instructionType::directive, 0)));

		inst.insert(make_pair(L"+", instruction(0, instructionType::$operator, 11)));	//add, pos(13)
		inst.insert(make_pair(L"-", instruction(0, instructionType::$operator, 11)));	//sub, neg(13)
		inst.insert(make_pair(L"*", instruction(0, instructionType::$operator, 12)));	//mul
		inst.insert(make_pair(L"/", instruction(0, instructionType::$operator, 12)));	//div
		inst.insert(make_pair(L"%", instruction(0, instructionType::$operator, 12)));	//mod
		//inst.insert(make_pair(L"**", instruction(0, instructionType::$operator, 14, associativity::right_associative)));	//pow
		inst.insert(make_pair(L"|", instruction(0, instructionType::$operator, 5)));	//bitwise or
		inst.insert(make_pair(L"&", instruction(0, instructionType::$operator, 7)));	//bitwise and
		inst.insert(make_pair(L"^", instruction(0, instructionType::$operator, 6)));	//bitwise xor
		inst.insert(make_pair(L"~", instruction(0, instructionType::$operator, 15, associativity::right_associative)));	//bitwise not
		inst.insert(make_pair(L"<<", instruction(0, instructionType::$operator, 10)));	//shift left
		inst.insert(make_pair(L">>", instruction(0, instructionType::$operator, 10)));	//logical shift right
		inst.insert(make_pair(L">>>", instruction(0, instructionType::$operator, 10)));	//arithmetic shift right
		inst.insert(make_pair(L"||", instruction(0, instructionType::$operator, 2)));	//bool or
		inst.insert(make_pair(L"&&", instruction(0, instructionType::$operator, 4)));	//bool and
		inst.insert(make_pair(L"^^", instruction(0, instructionType::$operator, 3)));	//bool xor
		inst.insert(make_pair(L"!", instruction(0, instructionType::$operator, 15, associativity::right_associative)));	//bool not
		inst.insert(make_pair(L"<", instruction(0, instructionType::$operator, 9)));	//bool less than
		inst.insert(make_pair(L">", instruction(0, instructionType::$operator, 9)));	//bool greater than
		inst.insert(make_pair(L"<=", instruction(0, instructionType::$operator, 9)));	//bool less or equal
		inst.insert(make_pair(L">=", instruction(0, instructionType::$operator, 9)));	//bool greater or equal
		inst.insert(make_pair(L"==", instruction(0, instructionType::$operator, 8)));	//bool equal
		inst.insert(make_pair(L"!=", instruction(0, instructionType::$operator, 8)));	//bool not equal
		inst.insert(make_pair(L",", instruction(0, instructionType::$operator, 1)));	//bool not equal
	}
};