#pragma once
#include "ScopeNode.h"
#include "TemplateArguments.h"
#include <vector>

struct TokenNode;
struct IdentifyListNode;
struct MemberListNode;
struct MethodNode;
struct TemplateParametersNode;
struct BaseClassListNode;
struct TemplateArguments;
struct ClassTypeNode;

struct ClassNode : ScopeNode
{
	enum LazyBool
	{
		lb_false,
		lb_true,
		lb_unknown,
	};
	TokenNode* m_modifier;
	TokenNode* m_keyword;
	IdentifyNode* m_category;
	TokenNode* m_colon;
	BaseClassListNode* m_baseList;
	TokenNode* m_leftBrace;
	TokenNode* m_rightBrace;
	TokenNode* m_semicolon;
	TemplateParametersNode* m_templateParametersNode;
	ClassTypeNode* m_typeNode;
	TemplateArguments m_templateArguments;
	TypeCategory m_typeCategory;
	bool m_override;
	bool m_arrayFlag;
public:
	ClassNode(TokenNode* keyword, IdentifyListNode* conceptList, IdentifyNode* name);
	void setTemplateParameters(TemplateParametersNode* templateParametersNode);
	void setMemberList(TokenNode* leftBrace, MemberListNode* memberList, TokenNode* rightBrace);
	bool needSubclassProxy(TemplateArguments* templateArguments);
	bool hasOverrideMethod(TemplateArguments* templateArguments);
	void collectOverrideMethods(std::vector<MethodNode*>& methodNodes, TemplateArguments* templateArguments);
	void generateCreateInstanceMethod(const char* methodName, MethodNode* constructor);
	void generateCreateArrayMethod(const char* methodName, MethodNode* constructor);
	TypeCategory getTypeCategory();
	virtual TypeNode* getTypeNode();
	virtual void getLocalName(std::string& name, TemplateArguments* templateArguments);
	virtual void collectTypes(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments);
	virtual void checkSemantic(TemplateArguments* templateArguments);
};