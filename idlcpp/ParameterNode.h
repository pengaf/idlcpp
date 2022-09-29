#pragma once

#include "SyntaxNodeImpl.h"

struct TokenNode;
struct TypeNameNode;
struct IdentifyNode;
struct MethodNode;
struct TemplateArguments;

struct ParameterNode : SyntaxNodeImpl
{
	TypeNameNode* m_typeName;
	IdentifyNode* m_name;
	TypeCompound m_typeCompound;
	ParameterPassing m_passing;
	TokenNode* m_defaultDenote;
public:
	ParameterNode(TypeNameNode* typeName, TypeCompound typeCompound, IdentifyNode* name);
public:
	void checkSemantic(TemplateArguments* templateArguments);
};
