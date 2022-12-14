#include "MethodNode.h"
#include "ParameterListNode.h"
#include "ParameterNode.h"
#include "TypeNameNode.h"
#include "IdentifyNode.h"
#include "ClassNode.h"
#include "ErrorList.h"
#include "RaiseError.h"
#include "TypeTree.h"
#include "Compiler.h"

#include <set>
#include <assert.h>

const char g_strPure[] = {" = 0 "};

MethodNode::MethodNode(IdentifyNode* name, TokenNode* leftParenthesis, ParameterListNode* parameterList, TokenNode* rightParenthesis, TokenNode* constant)
{
	m_nodeType = snt_method;
	m_modifier = 0;
	m_voidResult = 0;
	m_resultTypeName = 0;
	m_resultTypeCompound = tc_none;
	m_name = name;
	m_leftParenthesis = leftParenthesis;
	m_parameterList = parameterList;
	m_rightParenthesis = rightParenthesis;
	m_constant = constant;
	m_semicolon = 0;
	m_override = false;
	m_parameterCount = size_t(-1);
	m_firstDefaultParam = size_t(-1);
}

bool MethodNode::isStatic()
{
	return (0 != m_modifier && snt_keyword_static == m_modifier->m_nodeType);
}

bool MethodNode::isConstant()
{
	return (0 != m_constant && snt_keyword_const == m_constant->m_nodeType);
}

bool MethodNode::isVirtual()
{
	return (0 != m_modifier && 
		(snt_keyword_virtual == m_modifier->m_nodeType || 
		snt_keyword_abstract == m_modifier->m_nodeType));
}

bool MethodNode::isAbstract()
{
	return (0 != m_modifier && snt_keyword_abstract == m_modifier->m_nodeType);
}

size_t MethodNode::getParameterCount() const
{
	if (size_t(-1) == m_parameterCount)
	{
		size_t res = 0;
		ParameterListNode* list = m_parameterList;
		while (0 != list)
		{
			++res;
			list = list->m_parameterList;
		}
		m_parameterCount = res;
	}
	return m_parameterCount;
}

size_t MethodNode::getFirstDefaultParameter() const
{
	if (size_t(-1) == m_firstDefaultParam)
	{
		size_t paramCount = 0;
		size_t defaultCount = 0;
		ParameterListNode* list = m_parameterList;
		while (nullptr != list)
		{
			++paramCount;
			if (list->m_parameter->m_defaultDenote)
			{
				defaultCount = paramCount;
			}
			list = list->m_parameterList;
		}
		m_firstDefaultParam = paramCount - defaultCount;
	}
	return m_parameterCount;
}

void MethodNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	if (0 != m_resultTypeName)
	{
		m_resultTypeName->calcTypeNodes(enclosingTypeNode, templateArguments);
	}

	std::vector<ParameterNode*> parameterNodes;
	m_parameterList->collectParameterNodes(parameterNodes);
	auto it = parameterNodes.begin();
	auto end = parameterNodes.end();
	for (; it != end; ++it)
	{
		ParameterNode* parameterNode = *it;
		parameterNode->m_typeName->calcTypeNodes(enclosingTypeNode, templateArguments);
	}
}

void MethodNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);

	assert(snt_class == m_enclosing->m_nodeType);
	ClassNode* classNode = static_cast<ClassNode*>(m_enclosing);
	if(0 != m_resultTypeName)
	{
		TypeNode* typeNode = m_resultTypeName->getTypeNode(templateArguments);
		if (0 == typeNode)
		{
			return;
		}
		g_compiler.useType(typeNode, templateArguments, tc_none == m_resultTypeCompound ? tu_use_definition : tu_use_declaration, m_resultTypeName);
	}
	if(m_override)
	{
		if(!isVirtual())
		{
			RaiseError_InterfaceMethodIsNotVirtual(m_name);
		}
	}

	std::vector<ParameterNode*> parameterNodes;
	m_parameterList->collectParameterNodes(parameterNodes);
	checkParameterNames(parameterNodes);

	size_t parameterCount = parameterNodes.size();
	for(size_t i = 0; i < parameterCount; ++i)
	{
		parameterNodes[i]->checkSemantic(templateArguments);
	}
}
