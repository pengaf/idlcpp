#include "ClassNode.h"
#include "TypeNameNode.h"
#include "ScopeNameNode.h"
#include "ScopeNameListNode.h"
#include "IdentifyNode.h"
#include "IdentifyListNode.h"
#include "TypeNameListNode.h"

#include "TemplateParametersNode.h"
#include "TemplateClassInstanceNode.h"
#include "MemberNode.h"
#include "MemberListNode.h"
#include "MethodNode.h"
#include "ParameterListNode.h"
#include "ParameterNode.h"
#include "PropertyNode.h"
#include "FieldNode.h"
#include "NamespaceNode.h"
#include "ProgramNode.h"
#include "ErrorList.h"
#include "RaiseError.h"
#include "Options.h"
#include "Compiler.h"
#include "TypeTree.h"
#include "DelegateNode.h"

#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>


bool isDefaultConstructor(ClassNode* classNode, MethodNode* methodNode)
{
	if(classNode->m_name->m_str == methodNode->m_name->m_str)
	{
		if(0 == methodNode->getFirstDefaultParameter())
		{
			return true;
		}
	}
	return false;
}

void checkMemberNames(ClassNode* classNode, std::vector<MemberNode*>& memberNodes, TemplateArguments* templateArguments)
{
	std::set<IdentifyNode*, CompareIdentifyPtr> memberNames;

	size_t count = memberNodes.size();
	IdentifyNode* collisionNode = 0;
	for(size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		bool nameCollision = false;
		IdentifyNode* identify = memberNode->m_name;
		if (0 == identify)
		{
			//invalid operator has no name
			continue;
		}

		auto it = memberNames.find(identify);
		if (memberNames.end() != it)
		{
			collisionNode = *it;
			nameCollision = true;
		}
		else
		{
			memberNames.insert(identify);
		}
		if (nameCollision)
		{
			char buf[4096];
			sprintf_s(buf, "\'%s\' : member already defined at line %d, column %d", identify->m_str.c_str(),
				collisionNode->m_lineNo, collisionNode->m_columnNo);
			ErrorList_AddItem_CurrentFile(identify->m_lineNo,
				identify->m_columnNo, semantic_error_member_redefined, buf);
		}
		if (identify->m_str == classNode->m_name->m_str)
		{
			if (snt_method != memberNode->m_nodeType)
			{
				char buf[4096];
				sprintf_s(buf, "\'%s\' : class member name cannot equal to class name", identify->m_str.c_str());
				ErrorList_AddItem_CurrentFile(identify->m_lineNo,
					identify->m_columnNo, semantic_error_member_name_equal_to_class_name, buf);
			}
			else
			{
				MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
				if (0 != methodNode->m_modifier)
				{
					char buf[4096];
					sprintf_s(buf, "\'%s\' : constructor cannot be declared %s", identify->m_str.c_str(),
						g_keywordTokens[methodNode->m_modifier->m_nodeType - snt_begin_output - 1]);
					ErrorList_AddItem_CurrentFile(identify->m_lineNo,
						identify->m_columnNo, semantic_error_constructor_with_modifier, buf);
				}
				if (0 != methodNode->m_resultTypeName)
				{
					char buf[4096];
					sprintf_s(buf, "\'%s\' : constructor with return type", identify->m_str.c_str());
					ErrorList_AddItem_CurrentFile(identify->m_lineNo,
						identify->m_columnNo, semantic_error_constructor_with_return_type, buf);
				}
			}
		}
	}
}

static void ParseConceptList(
	IdentifyNode*& categoryNode,
	bool& sharedFlag,
	bool& arrayFlag,
	IdentifyListNode* conceptList)
{
	const char* s_categorys[] =
	{
		"primitive",
		"enumeration",
		"object",
		"string",
		"buffer",
		"enumerator",
		"instance_field",
		"static_field",
		"instance_property",
		"static_property",
		"instance_method",
		"static_method",
		"function_argument",
		"function_result",
		"primitive_type",
		"enumeration_type",
		"object_type",
		"type_alias",
		"name_space",
	};

	std::vector<IdentifyNode*> identifyNodes;
	conceptList->collectIdentifyNodes(identifyNodes);
	for (IdentifyNode* identifyNode : identifyNodes)
	{
		if (identifyNode->m_str == "shared")
		{
			sharedFlag = true;
		}
		else if (identifyNode->m_str == "array")
		{
			arrayFlag = true;
		}
		else if (0 == categoryNode)
		{
			for (int i = 0; i < sizeof(s_categorys) / sizeof(s_categorys[0]); ++i)
			{
				if (identifyNode->m_str == s_categorys[i])
				{
					categoryNode = identifyNode;
					break;
				}
			}
		}
	}
}


ClassNode::ClassNode(TokenNode* keyword, IdentifyListNode* conceptList, IdentifyNode* name)
{
	assert(snt_keyword_struct == keyword->m_nodeType 
		|| snt_keyword_class == keyword->m_nodeType 
		|| snt_keyword_delegate == keyword->m_nodeType);

	m_nodeType = snt_class;
	m_keyword = keyword;
	//m_conceptList = conceptList;
	m_name = name;
	m_modifier = 0;
	m_colon = 0;
	m_baseList = 0;
	m_leftBrace = 0;
	m_memberList = 0;
	m_rightBrace = 0;
	m_semicolon = 0;
	m_templateParametersNode = 0;
	m_typeNode = 0;
	m_category = 0;

	m_sharedFlag = false;
	m_arrayFlag = false;

	ParseConceptList(m_category, m_sharedFlag, m_arrayFlag, conceptList);
	m_override = false;

	m_typeCategory = object_type;
	if(0 != m_category)
	{
		if (m_category->m_str == "string")
		{
			m_typeCategory = string_type;
		}
		else if (m_category->m_str == "buffer")
		{
			m_typeCategory = buffer_type;
		}
	}	
}

void ClassNode::setTemplateParameters(TemplateParametersNode* templateParametersNode)
{
	assert(templateParametersNode);
	m_templateParametersNode = templateParametersNode;
}

void ClassNode::setMemberList(TokenNode* leftBrace, MemberListNode* memberList, TokenNode* rightBrace)
{
	m_leftBrace = leftBrace;
	m_memberList = memberList;
	m_rightBrace = rightBrace;
	m_memberList->initializeMembersEnclosing(this);
}


void ClassNode::extendInternalCode(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	if (m_templateParametersNode)
	{
		assert(0 == templateArguments);
		templateArguments = &m_templateArguments;
	}

	buildAdditionalMethods();

	std::vector<MemberNode*> memberNodes;
	m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_class:
			static_cast<ClassNode*>(memberNode)->extendInternalCode(m_typeNode, templateArguments);
			break;
		case snt_delegate:
			static_cast<DelegateNode*>(memberNode)->extendInternalCode(m_typeNode, templateArguments);
			break;
		}
	}

}

extern "C" extern int yytokenno;
extern "C" extern int yylineno;
extern "C" extern int yycolumnno;

void ClassNode::generateCreateInstanceMethod(const char* methodName, MethodNode* constructor)
{
	IdentifyNode* name = (IdentifyNode*)newIdentify(methodName);
	MethodNode* method = (MethodNode*)newMethod(name, 
		constructor->m_leftParenthesis, constructor->m_parameterList, 
		constructor->m_rightParenthesis, constructor->m_constant);
	method->m_semicolon = constructor->m_semicolon;
	ScopeNameNode* scopeName = (ScopeNameNode*)newScopeName(m_name, 0, 0, 0);
	ScopeNameListNode* scopeNameList = (ScopeNameListNode*)newScopeNameList(0, scopeName);
	TypeNameNode* typeName = (TypeNameNode*)newTypeName(scopeNameList);

	setMethodResult(method, typeName, m_sharedFlag ? tc_shared_ptr : tc_unique_ptr);
	TokenNode* modifier = (TokenNode*)newToken(snt_keyword_static);
	setMethodModifier(method, modifier);
	//if (constructor->m_filterNode)
	//{
	//	method->m_filterNode = (TokenNode*)newToken(constructor->m_filterNode->m_nodeType);
	//}
	method->m_enclosing = this;
	m_additionalMethods.push_back(method);
}

void ClassNode::generateCreateArrayMethod(const char* methodName, MethodNode* constructor)
{
	IdentifyNode* name = (IdentifyNode*)newIdentify(methodName);
	ParameterNode* parameter = (ParameterNode*)newParameter(newPrimitiveType(newToken(snt_keyword_unsigned), pt_uint), tc_none);
	setParameterName(parameter, newIdentify("count"));
	ParameterListNode* parameterList = (ParameterListNode*)newParameterList(0,0,parameter);
	MethodNode* method = (MethodNode*)newMethod(name, 
		constructor->m_leftParenthesis, parameterList, 
		constructor->m_rightParenthesis, constructor->m_constant);
	method->m_semicolon = constructor->m_semicolon;

	ScopeNameNode* scopeName = (ScopeNameNode*)newScopeName(m_name, 0, 0, 0);
	ScopeNameListNode* scopeNameList = (ScopeNameListNode*)newScopeNameList(0, scopeName);
	TypeNameNode* typeName = (TypeNameNode*)newTypeName(scopeNameList);
	setMethodResult(method, typeName, m_sharedFlag ? tc_shared_array : tc_unique_array);
	TokenNode* modifier = (TokenNode*)newToken(snt_keyword_static);
	setMethodModifier(method, modifier);
	//if (constructor->m_filterNode)
	//{
	//	method->m_filterNode = (TokenNode*)newToken(constructor->m_filterNode->m_nodeType);
	//}
	method->m_enclosing = this;
	m_additionalMethods.push_back(method);
}

void ClassNode::buildAdditionalMethods()
{
	int backupToken = yytokenno;
	int backupLine = yylineno;
	int backupColumn = yycolumnno;
	yytokenno = 0;
	yylineno = 0;
	yycolumnno = 0;

	MethodNode* constructor = nullptr;
	bool defaultConstructor = false;
	std::vector<MemberNode*> memberNodes;
	m_memberList->collectMemberNodes(memberNodes);
	size_t memberCount = memberNodes.size();
	for(size_t i = 0; i < memberCount; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		if(snt_method == memberNode->m_nodeType)
		{
			MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
			if(memberNode->m_name->m_str == m_name->m_str)
			{
				constructor = methodNode;
				defaultConstructor = isDefaultConstructor(this, methodNode);
				break;
			}
		}
	}

	if (constructor)
	{
		generateCreateInstanceMethod("New", constructor);
		bool isStruct = (snt_keyword_struct == m_keyword->m_nodeType);
		if (defaultConstructor && (m_arrayFlag || isStruct))
		{
			generateCreateArrayMethod("NewArray", constructor);
		}
	}

	//size_t count = constructors.size();
	//for(size_t i = 0; i < count; ++i)
	//{
	//	MethodNode* constructor = constructors[i];
	//	//if(!isValueType())
	//	//{
	//	//	GenerateCreateInstanceMethod("NewARC", constructor);
	//	//}
	//}
	//if(0 != defaultConstructor && !isIntrospectable())
	//{
	//	GenerateCreateArrayMethod("NewArray", defaultConstructor);
	//}
	yytokenno = backupToken;
	yylineno = backupLine;
	yycolumnno = backupColumn;
}


void ClassNode::collectOverrideMethods(std::vector<MethodNode*>& methodNodes, TemplateArguments* templateArguments)
{
	std::vector<MemberNode*> memberNodes;
	m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		if(snt_method == memberNode->m_nodeType)
		{
			MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
			if(methodNode->m_override)
			{
				methodNodes.push_back(methodNode);
			}
		}
	}
	std::vector<TypeNameNode*> baseTypeNameNodes;
	m_baseList->collectTypeNameNodes(baseTypeNameNodes);
	count = baseTypeNameNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = baseTypeNameNodes[i];
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (typeNode->isTemplateClassInstance())
		{
			TemplateClassInstanceTypeNode* templateClassInstanceTypeNode = static_cast<TemplateClassInstanceTypeNode*>(typeNode);
			templateClassInstanceTypeNode->m_classNode->collectOverrideMethods(methodNodes,
				&templateClassInstanceTypeNode->m_templateClassInstanceNode->m_templateArguments);
		}
		else
		{
			assert(typeNode->isClass() && !typeNode->isTemplateClass());
			ClassTypeNode* classTypeNode = static_cast<ClassTypeNode*>(typeNode);
			classTypeNode->m_classNode->collectOverrideMethods(methodNodes, 0);
		}
	}
}

bool ClassNode::hasOverrideMethod(TemplateArguments* templateArguments)
{
	std::vector<MemberNode*> memberNodes;
	m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		if(snt_method == memberNode->m_nodeType)
		{
			MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
			if(methodNode->m_override)
			{
				return true;
			}
		}
	}
	std::vector<TypeNameNode*> baseTypeNameNodes;
	m_baseList->collectTypeNameNodes(baseTypeNameNodes);
	count = baseTypeNameNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = baseTypeNameNodes[i];
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (typeNode->isTemplateClassInstance())
		{
			TemplateClassInstanceTypeNode* templateClassInstanceTypeNode = static_cast<TemplateClassInstanceTypeNode*>(typeNode);
			if (templateClassInstanceTypeNode->m_classNode->hasOverrideMethod(&templateClassInstanceTypeNode->m_templateClassInstanceNode->m_templateArguments))
			{
				return true;
			}
		}
		else
		{
			assert(typeNode->isClass() && !typeNode->isTemplateClass());
			ClassTypeNode* classTypeNode = static_cast<ClassTypeNode*>(typeNode);
			if (classTypeNode->m_classNode->hasOverrideMethod(0))
			{
				return true;
			}
		}
	}
	return false;
}

bool ClassNode::isAdditionalMethod(MethodNode* methodNode)
{
	auto it = m_additionalMethods.begin();
	auto end = m_additionalMethods.end();
	for(; it != end; ++it)
	{
		if (methodNode == *it)
		{
			return true;
		}
	}
	return false;
}

bool ClassNode::needSubclassProxy(TemplateArguments* templateArguments)
{
	if (m_override)
	{
		if (hasOverrideMethod(templateArguments))
		{
			return true;
		}
	}
	return false;
}

TypeCategory ClassNode::getTypeCategory()
{
	return m_typeCategory;
}

TypeNode* ClassNode::getTypeNode()
{
	return m_typeNode;
}

void ClassNode::getLocalName(std::string& name, TemplateArguments* templateArguments)
{
	if (m_templateParametersNode)
	{
		assert(templateArguments 
			&& templateArguments->m_classTypeNode->isTemplateClassInstance()
			&& templateArguments->m_className == m_name->m_str
			&& templateArguments->m_arguments.size() == m_templateArguments.m_arguments.size());
		name = static_cast<TemplateClassInstanceTypeNode*>(templateArguments->m_classTypeNode)->m_localName;
	}
	else
	{
		name = m_name->m_str;
	}
}

void ClassNode::collectTypes(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	assert(enclosingTypeNode);
	assert(0 == m_typeNode);
	if (0 != m_templateParametersNode)
	{		
		if (!m_templateParametersNode->checkSemantic())
		{
			return;
		}
	}
	switch (enclosingTypeNode->m_category)
	{
	case tc_namespace:
		m_typeNode = static_cast<NamespaceTypeNode*>(enclosingTypeNode)->addClass(this);
		break;
	case tc_class_type:
		if (0 == m_templateParametersNode)
		{
			m_typeNode = static_cast<ClassTypeNode*>(enclosingTypeNode)->addClass(this);
		}
		else
		{
			RaiseError_NestedTemplateClass(m_name);
		}
		break;
	default:
		assert(false);
	}
	if(m_typeNode)
	{
		if (m_templateParametersNode)
		{
			m_templateArguments.m_className = m_name->m_str;
			m_templateArguments.m_classTypeNode = m_typeNode;
			auto it = m_typeNode->m_parameterNodes.begin();
			auto end = m_typeNode->m_parameterNodes.end();
			for (; it != end; ++it)
			{
				TemplateParameterTypeNode* typeNode = *it;
				TemplateArgument arg;
				arg.m_name = typeNode->m_name;
				arg.m_typeNode = typeNode;
				m_templateArguments.m_arguments.push_back(arg);
			}
			assert(0 == templateArguments);
			templateArguments = &m_templateArguments;
		}
		if (m_memberList)
		{
			m_memberList->collectTypes(m_typeNode, templateArguments);
		}
	}
}

void ClassNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	if (m_templateParametersNode)
	{
		assert(0 == templateArguments);
		templateArguments = &m_templateArguments;
	}
	std::vector<TypeNameNode*> baseTypeNameNodes;
	m_baseList->collectTypeNameNodes(baseTypeNameNodes);
	std::vector<TypeNode*> baseTypeNodes;
	size_t baseCount = baseTypeNameNodes.size();
	for (size_t i = 0; i < baseCount; ++i)
	{
		TypeNameNode* typeNameNode = baseTypeNameNodes[i];
		typeNameNode->calcTypeNodes(enclosingTypeNode, templateArguments);
	}
	m_memberList->checkTypeNames(m_typeNode, templateArguments);


	size_t count = m_additionalMethods.size();
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = m_additionalMethods[i];
		//methodNode->checkTypeNames(m_typeNode, templateArguments);
		assert(methodNode->m_resultTypeName);
		methodNode->m_resultTypeName->calcTypeNodes(m_typeNode, templateArguments);
	}
}


void ClassNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);

	if (m_templateParametersNode)
	{
		if (0 == templateArguments)
		{
			return;
		}
	}
	assert(m_typeNode && m_typeNode->m_enclosing);
	std::vector<TypeNameNode*> baseTypeNameNodes;
	m_baseList->collectTypeNameNodes(baseTypeNameNodes);
	std::vector<TypeNode*> baseTypeNodes;
	size_t baseCount = baseTypeNameNodes.size();
	for (size_t i = 0; i < baseCount; ++i)
	{
		TypeNameNode* typeNameNode = baseTypeNameNodes[i];
		TypeNode* typeNode = typeNameNode->getTypeNode(templateArguments);
		if (0 != typeNode)
		{
			baseTypeNodes.push_back(typeNode);
			g_compiler.useType(typeNode, templateArguments, tu_use_definition, typeNameNode);
		}
	}

	//checkBaseTypes(this, baseTypeNameNodes, baseTypeNodes, templateArguments);

	std::vector<MemberNode*> memberNodes;
	m_memberList->collectMemberNodes(memberNodes);

	size_t memberCount = memberNodes.size();
	for (size_t i = 0; i < memberCount; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		memberNodes[i]->checkSemantic(templateArguments);
	}
	checkMemberNames(this, memberNodes, templateArguments);

	//if (string_type == m_typeCategory)
	//{
	//	bool findToString = false;
	//	bool findFromString = false;
	//	for (size_t i = 0; i < memberCount; ++i)
	//	{
	//		MemberNode* memberNode = memberNodes[i];
	//		if (snt_method == memberNode->m_nodeType)
	//		{
	//			MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
	//			if (!methodNode->isStatic())
	//			{
	//				if (methodNode->m_name->m_str == "toString")
	//				{
	//					findToString = true;
	//				}
	//				else if (methodNode->m_name->m_str == "fromString")
	//				{
	//					findFromString = true;
	//				}
	//			}
	//		}
	//	}
	//	if (!findToString)
	//	{
	//		RaiseError_MissingToString(m_name);
	//	}
	//	if (!findFromString)
	//	{
	//		RaiseError_MissingFromString(m_name);
	//	}
	//}
	//else if (buffer_type == m_typeCategory)
	//{
	//	bool findToBuffer = false;
	//	bool findFromBuffer = false;
	//	for (size_t i = 0; i < memberCount; ++i)
	//	{
	//		MemberNode* memberNode = memberNodes[i];
	//		if (snt_method == memberNode->m_nodeType)
	//		{
	//			MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
	//			if (!methodNode->isStatic())
	//			{
	//				if (methodNode->m_name->m_str == "toBuffer")
	//				{
	//					findToBuffer = true;
	//				}
	//				else if (methodNode->m_name->m_str == "fromBuffer")
	//				{
	//					findFromBuffer = true;
	//				}
	//			}
	//		}
	//	}
	//	if (!findToBuffer)
	//	{
	//		RaiseError_MissingToBuffer(m_name);
	//	}
	//	if (!findFromBuffer)
	//	{
	//		RaiseError_MissingFromBuffer(m_name);
	//	}
	//}

}
