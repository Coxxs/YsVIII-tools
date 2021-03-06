// Scene tool

#include "sceneTool.hh"
#include "sceneOpCodes.hh"
#include "sceneOpCodeNames.hh"
#include "vcalcOpCodes.hh"

#include <string>
#include <iostream>
#include <algorithm>
#include <iomanip>

#include <sstream>
#include <fstream>
#include <cstring>
#include <stack>
#include <map>


uint16_t read16(std::ifstream& input) {
	uint16_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

uint32_t read32(std::ifstream& input) {
	uint32_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

uint64_t read64(std::ifstream& input) {
	uint64_t data;
	input.read((char*)&data, (int)sizeof(data));
	return data;
}

// Checksum reverse by weaknespase
uint32_t checksum(const char* in, const uint32_t length, int last = 0){
	const char* end = in + length;
	int acc = last;
	while (in < end)
		acc = (acc * 33) ^ (unsigned char) *in++;
	return acc;
}

#define read_elem(fh, elem) fh.read((char*)&elem, (int)sizeof(elem))


void process_segment( std::ifstream& fh, SEGMENT_HEADER& segHead );

void print_hex(uint8_t *data, uint32_t size, std::stringstream &stream)
{
	for (uint32_t i = 0; i < size; ++i) {
		stream << " " << std::hex << std::setfill('0') << std::setw(2)
			<< (int)data[i] << std::dec;
	}
}

bool print_vcalc(uint8_t *data, uint32_t size, std::string &oString)
{
	bool hasNop = false;
	bool done = false;
	bool success = true;
	uint8_t *end = data + size;
	uint8_t *pCur = data;
	uint32_t vLast = 0;
	bool isIntLast = true;
	uint8_t nbVals = 0;
	std::string sCalc, sLast, sVal;
	std::stack<std::string> vStack;

	while ( pCur < end ) {

		if ( done )
			return false;

		uint16_t vcalcOp = *(uint16_t*)pCur;
		pCur += 2;

		if ( vStack.size() > 0 )
			sVal = vStack.top();

		hasNop = false;

		switch ( vcalcOp ) {
		case VCODE_NEQ:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " != " + sVal + ")";
			break;

		case VCODE_NOT:
			vStack.pop();
			sVal = "!" + sVal + "";
			break;

		case VCODE_MUL:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " * " + sVal + ")";
			break;

		case VCODE_DIV:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " / " + sVal + ")";
			break;

		case VCODE_IDIV:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " // " + sVal + ")";
			break;

		case VCODE_ADD:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " + " + sVal + ")";
			break;

		case VCODE_SUB:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " - " + sVal + ")";
			break;

		case VCODE_SRL:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " >> " + sVal + ")";
			break;

		case VCODE_GE:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " >= " + sVal + ")";
			break;

		case VCODE_GT:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " > " + sVal + ")";
			break;

		case VCODE_SLL:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " << " + sVal + ")";
			break;

		case VCODE_LE:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " <= " + sVal + ")";
			break;

		case VCODE_LT:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " < " + sVal + ")";
			break;

		case VCODE_EQ:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " == " + sVal + ")";
			break;

		case VCODE_AND:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " && " + sVal + ")";
			break;

		case VCODE_BAND:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " & " + sVal + ")";
			break;

		case VCODE_OR:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " || " + sVal + ")";
			break;

		case VCODE_BOR:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " | " + sVal + ")";
			break;

		case VCODE_BNOT:
			vStack.pop();
			sVal = "~" + sVal + "";
			break;

		case VCODE_XOR:
			vStack.pop(); sLast = vStack.top(); vStack.pop();
			sVal = "(" + sLast + " ^ " + sVal + ")";
			break;

		case VCODE_LDINT: {
			vLast = *(uint32_t*)pCur;
			pCur += 4;

			sVal = std::to_string(vLast);
			break;
		}

		case VCODE_LDFLT: {
			vLast = *(uint32_t*)pCur;
			float value = *(float*)pCur;
			pCur += 4;

			sVal = std::to_string(value);
			break;
		}

		case VCODE_END:
			hasNop = true;
			done = true;
			break;

		case VCODE_LDVAR: {
			vStack.pop();
			sVal = std::string(isIntLast ? "iVar" : "fVar") + "[" + std::to_string(vLast) + "]";
			break;
		}

		// Don't handle nops
		// case VCODE_NOP:
		// 	hasNop = true;
		// 	break;

		case VCODE_ABS:
			vStack.pop();
			sVal = "|" + sVal + "|";
			break;

		case VCODE_F2I:
			vStack.pop();
			sVal = "(int)" + sVal;
			break;

		case VCODE_I2F:
			vStack.pop();
			sVal = "(float)" + sVal;
			break;

		case VCODE_RAND:
			sVal = "RAND";
			break;

		case VCODE_FRAND:
			sVal = "FRAND";
			break;

		case VCODE_NEG:
			vStack.pop();
			sVal = "-" + sVal;
			break;

		default:
			// std::cerr << "VCALC UNSUPPORTED OPERATION : 0x" << std::hex << vcalcOp << std::dec << std::endl;
			// exit(1);
			return false;
		}

		if ( !hasNop )
			vStack.push(sVal);
	}

	if ( vStack.size() != 1 ) {
		std::cerr << "VCALC STACK ERROR : " << vStack.size() << std::endl;
		std::cerr << "TOP : " << vStack.top() << std::endl;
		// vStack.pop();
		// std::cerr << "TOP : " << vStack.top() << std::endl;
		std::stringstream ss;
		print_hex(data, size, ss);
		std::cerr << ss.str() << std::endl;
		// exit(1);
		return false;
	}

	if ( done )
		oString = vStack.top();

	return done;
}


bool minimize = false;
bool enable_vcalc = false;


int main(int argc, char *argv[])
{
	// std::string ss;
	// uint8_t tData[] = { 0x1A, 0, 1, 0, 0, 0, 0x1F, 0, 0x1D, 0 };
	// print_vcalc( tData, sizeof(tData), ss ); std::cout << ss << std::endl;
	//exit(0);
	std::string iPath;

	for ( int i = 1; i < argc; i++ ) {

		std::string arg = argv[i];

		if ( arg == "-m" ) {
			std::cerr << "Minimizing output" << std::endl;
			minimize = true;
		}

		else if ( arg == "-t" ) {
			std::cerr << "Enabling VCALC translation" << std::endl;
			enable_vcalc = true;
		}

		else if ( iPath.size() < 1 ) {
			iPath = arg;
		}
	}

	if ( argc < 2 || iPath.size() < 1 ) {
		std::cerr << "Usage : " << argv[0]
			<< " <byteScript> (-m Minimize, -t VCALC translation)" << std::endl;
		return -1;
	}

	std::ifstream iFile( iPath.c_str(), std::ios_base::binary );

	if ( !iFile.is_open() ) {
		std::cerr << iPath << " not found" << std::endl;
		return -2;
	}


	FILE_HEADER_V2 headerV2;
	read_elem( iFile, headerV2 );

	if ( strncmp( headerV2.magic, "YS7_SCP", 8 ) ) {
		std::cerr << "Invalid input file" << std::endl;
		iFile.close();
		return 1;
	}

	SEGMENT_HEADER segHeads[ headerV2.segs_count ];
	read_elem( iFile, segHeads );

	for ( int i = 0; i < headerV2.segs_count; i++ ) {
		process_segment( iFile, segHeads[i] );
	}

	iFile.close();

	return 0;
}

inline bool isJapOpCode( uint16_t code )
{
	return (0x8000 <= code && code <= 0x8181);
}

inline bool isOpCode( uint16_t code )
{
	return (0x8000 <= code && code <= 0x8182);
}

std::map<uint32_t, uint32_t> labelMap;

void process_segment( std::ifstream& fh, SEGMENT_HEADER& segHead )
{
	fh.seekg( segHead.offset );

	uint32_t size = segHead.size;
	char *segment = (char*)malloc( size );

	fh.read( segment, size );

	std::cout << std::endl;
	std::cout << "0x" << std::hex << (int)segHead.offset << std::dec << std::endl;
	std::cout << segHead.name << std::endl;
	std::cout << "Size : 0x" << std::hex << (int)segHead.size << std::dec << std::endl;

	char *pSeg = segment, *pEnd = segment + size;
	bool hasJump = false;
	GENERIC_ARG arg;
	uint32_t labelIdx = 0;

	std::stringstream ss;
	std::map<uint32_t, std::string> lines;

	while ( true ) {

		const uint32_t lineOffset = (uint32_t)(pSeg - segment);

		if ( hasJump ) {
			uint32_t index = 0;
			const uint32_t labelOffset = (unsigned int)((pSeg - segment) + arg.iVal);

			if ( labelMap.count( labelOffset ) <= 0 ) {
				index = labelIdx;
				labelMap[ labelOffset ] = labelIdx;

				if ( arg.iVal < 0 ) {

					if ( lines.count( labelOffset ) > 0 ) {

						std::string oldLine = lines[ labelOffset ];
						std::string label = "label" + std::to_string(labelIdx) + ": ";
						std::string newLine;

						if ( !minimize ) {
							size_t begPos = oldLine.find(":");
							std::string relOffset = oldLine.substr( 0, begPos+2 );
							newLine = relOffset + label + oldLine.substr( begPos+2 );
						}
						else {
							size_t begPos = oldLine.find("\n");
							newLine = oldLine.substr(0, begPos+1) + label + oldLine.substr(begPos+1);
						}

						lines[ labelOffset ] = newLine;
					}

					else {
						std::cerr << "ERROR : invalid label reference to 0x" << std::hex << labelOffset << std::dec << std::endl;
					}
				}

				labelIdx++;
			}

			else
				index = labelMap[ labelOffset ];

			ss << " / label" << index;
		}

		if ( pSeg >= (pEnd-1) ) {
			lines[ lineOffset ] = ss.str();
			break;
		}

		ss << std::endl;

		OpCode opcode = (OpCode)*((uint16_t*)pSeg);

		if ( !isOpCode( opcode ) ) {
			std::cerr << std::endl << "ERROR : bad opcode 0x"
				<< std::hex << (int)opcode
				<< " at offset 0x" << (pSeg - segment + segHead.offset)
				<< std::dec;
			// return;
		}

		else if ( !isJapOpCode( opcode ) ) {
			std::cerr << std::endl << "WARNING : unsupported english opcode 0x"
				<< std::hex << (int)opcode
				<< " at offset 0x" << (pSeg - segment + segHead.offset)
				<< std::dec;
			// return;
		}

		if ( !minimize ) {
			ss << "0x" << std::hex << std::setfill('0') << std::setw(4)
				<< (int)(pSeg - segment) << ": "
				<< std::dec;
		}

		const uint32_t curOffset = (unsigned int)(pSeg - segment);

		if ( labelMap.count( curOffset ) ) {
			ss << "label" << labelMap[ curOffset ] << ": ";
		}

		uint32_t opcode_idx = opcode - OPCODE_exit;
		std::string opcode_name;

		if ( opcode_idx < sizeof(opCodeNames)/sizeof(char*) ) {
			opcode_name = opCodeNames[ opcode_idx ];
			ss << opcode_name;
		}
		else {
			ss << std::hex << "0x" << opcode << std::dec;
		}

		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		pSeg += sizeof(uint16_t);
		// std::cerr << std::endl << "sizeof(uint16_t) = " << sizeof(uint16_t);
		// std::cerr << std::endl << "pSeg = " << std::hex << (int)pSeg << std::dec;
		// std::cerr << std::endl << "opcode = " << std::hex << (int)opcode << std::dec;

		// if ( opcode == OPCODE_exit ) {
		// 	ss << std::endl;
		// 	return;
		// }

		bool is_opcode = false;
		uint16_t nargs = 0;

		switch (opcode) {
		case OPCODE_if:
		case OPCODE_elseIf:
		case OPCODE_else:
		case OPCODE_break:
		case OPCODE_while:
		case OPCODE_case:
		case OPCODE_default:
		case OPCODE_Execute:
		case OPCODE_8085:
			hasJump = true;
			break;

		default:
			hasJump = false;
		};


		while ( !is_opcode && (pSeg < (pEnd-1)) ) {

			const char *pArg = pSeg;
			GENERIC_ARG cur = *((GENERIC_ARG*)pSeg);
			DataTag tag = (DataTag)cur.tag;
			is_opcode = isOpCode( tag );
			uint32_t tag_val;

			if ( is_opcode || ++nargs >= 16 ) {
				// std::cerr << std::endl << "New OPCODE detected";
				break;
			}

			else {
				// std::cerr << "New ARG detected" << std::endl;
				// pSeg += sizeof(uint16_t);

				// tag_val = *((uint32_t*)pSeg);
				// pSeg += sizeof(uint32_t);

				pSeg += sizeof(GENERIC_ARG);

				// ss << " ";
			}

			arg = cur;

			// std::cerr << "Switch ARG" << std::endl;

			switch ( tag ) {

			case INT_TAG:
				ss << ", #" << arg.iVal;

				if ( !minimize ) {
					ss << " (@ 0x" << std::hex << std::setfill('0') << std::setw(4)
							<< ((int)(pSeg - segment) + (int)arg.iVal) << std::dec << ")";
				}
				break;

			case FLOAT_TAG:
				// ss << ", f" << arg.fVal;
				ss << ", .0x" << std::hex << arg.uVal << std::dec;
				ss << " (~ " << arg.fVal << ")";
				break;

			case STRING_TAG: {
				ss << ", s\"";
				// ss << ", "<<arg.uVal<<"\"";

				uint32_t len = arg.uVal;
				char data[len+1];

				strncpy(data, pSeg, len);
				data[len] = 0;
				pSeg += len;

				// TODO : find unambiguous string delimiters
				ss << data << "\"";
				break;
			}

			case POPUP_TAG: {
				// ss << ", popup (" << arg.uVal << ":";
				ss << ", p";

				POPUP_ARG popup = *(POPUP_ARG*)pArg;
				uint32_t nLines = popup.nLines;
				uint32_t len = popup.len;
				pSeg += sizeof(uint32_t);

				uint32_t args[ nLines ];

				for (uint32_t i = 0; i < nLines; i++) {
					args[i] = *((uint32_t*)pSeg);
					pSeg += sizeof(uint32_t);

					// ss << " " << args[i] << " ;";
				}

				// ss << " \"";
				ss << "\"";

				char data[len+1];

				strncpy(data, pSeg, len);
				data[len] = 0;
				pSeg += len;

				// TODO : find unambiguous string delimiters
				ss << data << "\"";
				ss << " (";

				for (uint32_t i = 0; i < nLines; i++) {
					ss << " " << args[i];
				}
				ss << " )";

				// ss << data << "\" )";
				break;
			}

			case VCALC_TAG: {
				uint32_t count = arg.uVal;

				// if ( count != 8 ) {
				// 	std::cerr << std::endl << "WARNING : 0x82E0 ARG size = " << count;
				// }

				if ( count > (pEnd - pSeg) ) {
					std::cerr << std::endl << "FATAL ERROR, segment aborted";
					ss << std::endl << "FATAL ERROR, segment aborted";
					return;
				}

				//print_hex((uint8_t*)pSeg, count, ss);
				std::string sCalc;
				bool success = false;

				if ( enable_vcalc )
					success = print_vcalc((uint8_t*)pSeg, count, sCalc);

				if ( success )
					ss << ", vCALC (" + sCalc + ")";

				else {
					ss << ", o";
					print_hex((uint8_t*)pSeg, count, ss);
				}

				pSeg += count;
				break;
			}

			default:
				std::cerr << std::endl
					<< "ERROR : Unknown argument 0x" << std::hex << (int)tag
					<< " at offset 0x" << (pArg - segment + segHead.offset)
					<< std::dec << ", stopping argument parsing";

				is_opcode = true;
				pSeg = (char*)pArg;
				break;

				// ss << ", " << std::hex << "0x" << (int)tag << " : 0x" << arg.uVal << std::dec;
				// break;
			}
		}

		lines[ lineOffset ] = ss.str();
		ss.clear();
		ss.str("");
	}

	for ( auto &line : lines ) {
		std::cout << line.second;
	}

	std::cout << std::endl << "0x" << std::hex << std::setfill('0') << std::setw(4)
		<< (int)(pSeg - segment) << ": "
		<< std::dec;

	if ( labelMap.count( (unsigned int)(pSeg - segment) ) ) {
		std::cout << "label" << labelMap[ (unsigned int)(pSeg - segment) ] << ": ";
	}

	labelMap.clear();

	std::cout << std::endl;
}
