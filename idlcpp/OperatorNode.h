#pragma once
#include "MemberNode.h"

enum OperatorCategory
{
	assgin_operator,
	cast_operator,
};

struct OperatorNode : MemberNode
{
public:
	TokenNode* m_keyword;
	OperatorCategory m_operatorCategory;
public:
	OperatorNode(TokenNode* keyword, OperatorCategory operatorCategory) : m_keyword(keyword), m_operatorCategory(operatorCategory)
	{
		m_nodeType = snt_operator;
	}
};

struct AssignOperatorNode : OperatorNode
{
public:
	TokenNode* m_sign;
	TokenNode* m_leftParenthesis;
	TypeNameNode* m_paramTypeName;
	ParameterPassing m_paramPassing;
	TokenNode* m_rightParenthesis;
	TokenNode* m_semicolon;
public:
	AssignOperatorNode(TokenNode* keyword, TokenNode* sign, TokenNode* leftParenthesis,
		TypeNameNode* paramTypeName, ParameterPassing paramPassing, TokenNode* rightParenthesis, TokenNode* semicolon);
public:
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};

struct CastOperatorNode : OperatorNode
{
public:
	TypeNameNode* m_resultTypeName;
	TokenNode* m_leftParenthesis;
	TokenNode* m_rightParenthesis;
	TokenNode* m_constant;
	TokenNode* m_semicolon;
public:
	CastOperatorNode(TokenNode* keyword, TypeNameNode* resultTypeName, TokenNode* leftParenthesis,
		TokenNode* rightParenthesis, TokenNode* constant, TokenNode* semicolon);
public:
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};