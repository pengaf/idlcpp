#include "BaseClassNode.h"

BaseClassNode::BaseClassNode(TypeNameNode* typeName, TokenNode* filter)
{
	m_nodeType = snt_base_class;
	m_typeName = typeName;
	m_filterNode = filter;
}

bool BaseClassNode::isNoCode()
{
	return (m_filterNode && snt_keyword_nocode == m_filterNode->m_nodeType);
}

bool BaseClassNode::isNoMeta()
{
	return (m_filterNode && snt_keyword_nometa == m_filterNode->m_nodeType);
}
