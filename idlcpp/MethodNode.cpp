#include "MethodNode.h"
#include "VariableTypeListNode.h"
#include "VariableTypeNode.h"
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
	m_resultList = 0;
	m_name = name;
	m_leftParenthesis = leftParenthesis;
	m_parameterList = parameterList;
	m_rightParenthesis = rightParenthesis;
	m_constant = constant;
	m_semicolon = 0;
	m_override = false;
	m_resultCount = size_t(-1);
	m_parameterCount = size_t(-1);
	m_defaultParameterCount = size_t(-1);
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

size_t MethodNode::getResultCount() const
{
	if (size_t(-1) == m_resultCount)
	{
		size_t res = 0;
		VariableTypeListNode* list = m_resultList;
		while (0 != list)
		{
			++res;
			list = list->m_variableTypeList;
		}
		m_resultCount = res;
	}
	return m_resultCount;
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

size_t MethodNode::getDefaultParameterCount() const
{
	if (size_t(-1) == m_defaultParameterCount)
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
		m_defaultParameterCount = defaultCount;
	}
	return m_defaultParameterCount;
}

void MethodNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	std::vector<VariableTypeNode*> resultNodes;
	m_resultList->collectVariableTypeNodes(resultNodes);
	for (VariableTypeNode* resultNode : resultNodes)
	{
		resultNode->m_typeName->calcTypeNodes(enclosingTypeNode, templateArguments);
	}

	std::vector<ParameterNode*> parameterNodes;
	m_parameterList->collectParameterNodes(parameterNodes);
	for (ParameterNode* parameterNode : parameterNodes)
	{
		parameterNode->m_typeName->calcTypeNodes(enclosingTypeNode, templateArguments);
	}
}

void MethodNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);

	assert(snt_class == m_enclosing->m_nodeType);
	ClassNode* classNode = static_cast<ClassNode*>(m_enclosing);
	if(m_override)
	{
		if(!isVirtual())
		{
			RaiseError_InterfaceMethodIsNotVirtual(m_name);
		}
	}

	std::vector<VariableTypeNode*> resultNodes;
	m_resultList->collectVariableTypeNodes(resultNodes);
	
	bool outputParam = (m_voidResult != nullptr);
	for (VariableTypeNode* resultNode : resultNodes)
	{
		resultNode->checkSemantic(templateArguments, outputParam);
		outputParam = true;
	}

	int defaultParamOffset = getParameterCount() - getDefaultParameterCount();
	std::vector<ParameterNode*> parameterNodes;
	m_parameterList->collectParameterNodes(parameterNodes);	
	checkParameterNames(parameterNodes);
	for (ParameterNode* parameterNode : parameterNodes)
	{
		parameterNode->checkSemantic(templateArguments);
		--defaultParamOffset;
		if (defaultParamOffset < 0 && !parameterNode->m_defaultDenote)
		{
			RaiseError_MissingDefaultParameter(parameterNode->m_name);
		}
	}
}
