#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_KEMX

#include "node-info.hpp"
#include "pfm.hpp"

using std::string;
using std::vector;


const string whitespace_seq(" \t\n\r,");

bool is_whitespace(char c)
{
	int whitespaceSeqLength = whitespace_seq.length();

	for (int k = 0; k < whitespaceSeqLength; k++)
	{
		if (c == whitespace_seq[k])
		{
			return true;
		}
	}

	return false;
}

int is_whitespace_intv(int c)
{
	return is_whitespace(c);
}

inline std::string ni_ltrim(const std::string& is)
{
	std::string s(is);
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c) {return !is_whitespace_intv(c); }));
	return s;
}

// trim from end
inline std::string ni_rtrim(const std::string& is)
{
	std::string s(is);
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int c) {return !is_whitespace_intv(c); }).base(), s.end());
	return s;
}

// trim from both ends
inline std::string ni_trim(const std::string& is)
{
	return ni_ltrim(ni_rtrim(is));
}

//std::string ttrim(const std::string& pString)
//{
//	const size_t beginStr = pString.find_first_not_of(whitespaceSeq);
//
//	if (beginStr == std::string::npos)
//	{
//		// no content
//		return "";
//	}
//
//	const size_t endStr = pString.find_last_not_of(whitespaceSeq);
//	const size_t range = endStr - beginStr + 1;
//
//	return pString.substr(beginStr, range);
//}

node_info::node_info()
{
}

node_info::~node_info()
{
}

int node_info::parseBlock(std::string& exp, std::vector<shared_ptr<node_info> >& list, int level)
{
	int expLen = exp.length();
	int blockLen = 1;
	int idxLeft = 0;

	while (is_whitespace(exp.at(idxLeft)) && idxLeft < (expLen - 1))
	{
		idxLeft++;
	}

	if (exp.at(idxLeft) != '[')
	{
		string tstr = "parse error. expected '[' " + exp;
      mws_throw IllegalStateException(tstr);
	}

	int idxRight = -1;
	int prevNameIdx = idxLeft + 1;
	int idx = idxLeft + 1;
	bool isSimple = true, endFound = false;
	//string exp0 = "[ dw[ dw[ dws[ url[ img[ src[*_*.jpg] [xxx] ] ] ] url[ a [ href [*kami_*_*.php] ] ] ] url[xxx-url] ] ]";
	// "[file [name ["xxx.kemx"], size [188], last mod time [2012-09-23], type [jpeg] ] ]"

	for (; idx < expLen; idx++)
	{
		char c = exp.at(idx);

		if (c == '[')
		{
			string newExp = exp.substr(idx, exp.length() - idx - 1);
			string subexp = exp.substr(prevNameIdx, idx - prevNameIdx);
			string name = ni_trim(subexp);//.trim();

			//if (name.length() == 0)
			//{
			//	mws_throw new IllegalStateException("invalid name: " + exp);
			//}

			shared_ptr<node_info> kv(new node_info());

			kv->name = name;
			list.push_back(kv);

			int blockSize = parseBlock(newExp, kv->list, level + 1);
			//kv->val = ttrim(exp.substr(idx + 1, idx + blockSize - 1 - (idx + 1)));//.trim();
			idx += blockSize;
			prevNameIdx = idx;
			isSimple = false;
		}
		else if (c == ']')
		{
			idxRight = idx;
			endFound = true;
			break;
		}
	}

	if (!endFound)
	{
		string tstr = "parse error. unclosed exp " + exp;
      mws_throw IllegalStateException(tstr);
	}

	string s;
	blockLen += idx;
	exp = ni_trim(exp.substr(idxLeft + 1, idxRight - (idxLeft + 1)));//.trim();

	if (isSimple)
	{
		shared_ptr<node_info> kv(new node_info());

		kv->name = exp;
		list.push_back(kv);
		s = "is";
	}
	else
	{
		s = "ns";
	}

	//tracenl("%s, l: %d %s", s.c_str(), blockLen, exp.c_str());

	return blockLen;
}

void print_level_offset(int ilevel)
{
	for (int k = 0; k < ilevel; k++)
	{
		trc("\t");
	}
}

void node_info::printList(vector<shared_ptr<node_info> >& list, int ilevel)
{
	int size = list.size();

	for (int k = 0; k < size; k++)
	{
		shared_ptr<node_info> kv = list[k];
		bool new_line_block = (kv->list.size() > 0);

		if (kv->name.length() > 0)
		{
			print_level_offset(ilevel);
			mws_print("%s", kv->name.c_str());
		}

		if (new_line_block)
		{
			trc("\n");
			print_level_offset(ilevel);
			trc("[\n");
		}
		else
		{
			trc("[ ");
		}

		if (kv->list.size() > 0)
		{
			printList(kv->list, ilevel + 1);
		}

		if (new_line_block)
		{
			trc("\n");
			print_level_offset(ilevel);
			trc("],\n");
		}
		else
		{
			trc(" ],");
		}
	}
}

#endif
