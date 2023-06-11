#include "OperatorNode.h"
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

AssignOperatorNode::AssignOperatorNode(TokenNode* keyword, TokenNode* sign, TokenNode* leftParenthesis,
	TypeNameNode* paramTypeName, ParameterPassing paramPassing, TokenNode* rightParenthesis, TokenNode* semicolon) :
	OperatorNode(keyword, assgin_operator)
{
	m_sign = sign;
	m_leftParenthesis = leftParenthesis;
	m_paramTypeName = paramTypeName;
	m_paramPassing = paramPassing;
	m_rightParenthesis = rightParenthesis;
	m_semicolon = semicolon;
}

void AssignOperatorNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	m_paramTypeName->calcTypeNodes(enclosingTypeNode, templateArguments);
}

void AssignOperatorNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);
	assert(snt_class == m_enclosing->m_nodeType);
	ClassNode* classNode = static_cast<ClassNode*>(m_enclosing);
	TypeNode* typeNode = m_paramTypeName->getTypeNode(templateArguments);
	if (nullptr != typeNode)
	{
		g_compiler.useType(typeNode, templateArguments, pp_value == m_paramPassing ? tu_use_definition : tu_use_declaration, m_paramTypeName);
	}
}

CastOperatorNode::CastOperatorNode(TokenNode* keyword, TypeNameNode* resultTypeName, TokenNode* leftParenthesis,
	TokenNode* rightParenthesis, TokenNode* constant, TokenNode* semicolon) :
	OperatorNode(keyword, cast_operator)
{
	m_resultTypeName = resultTypeName;
	m_leftParenthesis = leftParenthesis;
	m_rightParenthesis = rightParenthesis;
	m_constant = constant;
	m_semicolon = semicolon;
}

void CastOperatorNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	m_resultTypeName->calcTypeNodes(enclosingTypeNode, templateArguments);
}

void CastOperatorNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);
	assert(snt_class == m_enclosing->m_nodeType);
	ClassNode* classNode = static_cast<ClassNode*>(m_enclosing);
	if (0 != m_resultTypeName)
	{
		TypeNode* typeNode = m_resultTypeName->getTypeNode(templateArguments);
		if (nullptr != typeNode)
		{
			g_compiler.useType(typeNode, templateArguments, tu_use_definition, m_resultTypeName);
		}
	}
}
