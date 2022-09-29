#pragma once
#include "MemberNode.h"

struct SyntaxNodeImpl;
struct TokenNode;
struct ParameterListNode;
struct ParameterNode;
struct TypeNameNode;
struct ClassNode;

struct DelegateNode : MemberNode
{
public:
	TokenNode* m_keyword;
	TypeNameNode* m_resultTypeName;
	TypeCompound m_resultTypeCompound;
	TokenNode* m_leftParenthesis;
	ParameterListNode* m_parameterList;
	TokenNode* m_rightParenthesis;
	TokenNode* m_semicolon;
	mutable size_t m_parameterCount;
	ClassNode* m_classNode;
public:
	DelegateNode(IdentifyNode* name, TokenNode* leftParenthesis, ParameterListNode* parameterList, TokenNode* rightParenthesis, TokenNode* semicolon);
	size_t getParameterCount() const;
	void extendInternalCode(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void collectTypes(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};
