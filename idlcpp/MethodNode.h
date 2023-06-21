#pragma once
#include "MemberNode.h"

struct SyntaxNodeImpl;
struct TokenNode;
struct ParameterListNode;
struct VariableTypeListNode;
struct TypeNameNode;

struct MethodNode : MemberNode
{
public:
	TokenNode* m_modifier;//static virtual abstract
	TokenNode* m_voidResult;
	VariableTypeListNode* m_resultList;
	TokenNode* m_leftParenthesis;
	ParameterListNode* m_parameterList;
	TokenNode* m_rightParenthesis;
	TokenNode* m_constant;
	TokenNode* m_semicolon;
	bool m_override;
	mutable size_t m_resultCount;
	mutable size_t m_parameterCount;
	mutable size_t m_defaultParameterCount;
public:
	MethodNode(IdentifyNode* name, TokenNode* leftParenthesis, ParameterListNode* parameterList, TokenNode* rightParenthesis, TokenNode* constant);
	bool isStatic();
	bool isVirtual();
	bool isAbstract();
	bool isConstant();
	TypeCompound getResultTypeCompound();
	size_t getResultCount() const;
	size_t getParameterCount() const;
	size_t getDefaultParameterCount() const;
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};
