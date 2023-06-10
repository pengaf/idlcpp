#include "Utility.h"
#include "SourceFile.h"
#include <algorithm>

SourceFile::~SourceFile()
{
	size_t count = m_embededCodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		delete m_embededCodes[i];
	}
}

struct CompareEmbededCode
{
	bool operator () (const EmbededCode* lhs, const EmbededCode* rhs) const
	{
		return lhs->m_tokenNo < rhs->m_tokenNo;
	}
};

EmbededCode* SourceFile::getEmbededCode(int tokenNo)
{
	EmbededCode dummy;
	dummy.m_tokenNo = tokenNo;
	auto it = std::lower_bound(m_embededCodes.begin(), m_embededCodes.end(), &dummy, CompareEmbededCode());
	if (it != m_embededCodes.end() && tokenNo == (*it)->m_tokenNo)
	{
		return *it;
	}
	return nullptr;
}

void SourceFile::addEmbededCodeBlock(const char* str, int tokenNo)
{
	EmbededCode* embededCode = new EmbededCode;
	embededCode->m_code = str;
	embededCode->m_tokenNo = tokenNo;
	m_embededCodes.push_back(embededCode);
}

void SourceFile::outputEmbededCodes(FILE* file, int tokenNo)
{
	for (; m_currentEmbededCode < m_embededCodes.size(); ++m_currentEmbededCode)
	{
		EmbededCode* embededCode = m_embededCodes[m_currentEmbededCode];
		if (embededCode->m_tokenNo <= tokenNo)
		{
			if (!embededCode->m_code.empty())
			{
				if (isNumAlpha_(GetLastWrittenChar()) 
					&& isNumAlpha_(embededCode->m_code.c_str()[0]))
				{
					writeSpaceToFile(file);
				}
				writeStringToFile(embededCode->m_code.c_str(), embededCode->m_code.length(), file);
			}
		}
		else
		{
			break;
		}
	}
}
