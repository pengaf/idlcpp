#pragma once

#include "MemberNode.h"

struct TypeNameNode;

struct GetterNode : SyntaxNodeImpl
{
	TokenNode* m_keyword;
	IdentifyNode* m_nativeName;
	TypeCompound m_typeCompound;
	ParameterPassing m_passing;
public:
	GetterNode(TokenNode* keyword, TypeCompound typeCompound);
};

struct SetterNode : SyntaxNodeImpl
{
	TokenNode* m_keyword;
	IdentifyNode* m_nativeName;
	TypeCompound m_typeCompound;
	ParameterPassing m_passing;
public:
	SetterNode(TokenNode* keyword, TypeCompound typeCompound);
};

struct PropertyNode : MemberNode
{
	TokenNode* m_modifier;
	TypeNameNode* m_typeName;
	GetterNode* m_get;
	SetterNode* m_set;
	PropertyCategory m_propertyCategory;
public:
	PropertyNode(TypeNameNode* typeName, IdentifyNode* name, PropertyCategory category);
	PropertyCategory getCategory();
	bool isStatic();
	bool isSimple();
	bool isArray();
	bool isCollection();
	void setGetter(GetterNode* getter);
	void setSetter(SetterNode* setter);

	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};