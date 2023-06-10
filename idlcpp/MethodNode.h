#pragma once
#include "MemberNode.h"

struct SyntaxNodeImpl;
struct TokenNode;
struct ParameterListNode;
struct ParameterNode;
struct TypeNameNode;

struct MethodNode : MemberNode
{
public:
	TokenNode* m_modifier;//static virtual abstract
	TokenNode* m_voidResult;
	TypeNameNode* m_resultTypeName;
	TypeCompound m_resultTypeCompound;
	TokenNode* m_leftParenthesis;
	ParameterListNode* m_parameterList;
	TokenNode* m_rightParenthesis;
	TokenNode* m_constant;
	TokenNode* m_semicolon;
	bool m_override;
	bool m_additional;
	mutable size_t m_parameterCount;
	mutable size_t m_firstDefaultParam;
public:
	MethodNode(IdentifyNode* name, TokenNode* leftParenthesis, ParameterListNode* parameterList, TokenNode* rightParenthesis, TokenNode* constant);
	bool isStatic();
	bool isVirtual();
	bool isAbstract();
	bool isConstant();
	TypeCompound getResultTypeCompound();
	size_t getParameterCount() const;
	size_t getFirstDefaultParameter() const;
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};
