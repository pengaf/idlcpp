#include "ClassNode.h"
#include "TypeNameNode.h"
#include "ScopeNameNode.h"
#include "ScopeNameListNode.h"
#include "IdentifyNode.h"
#include "IdentifyListNode.h"
#include "TypeNameListNode.h"
#include "BaseClassNode.h"
#include "BaseClassListNode.h"

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

#include <assert.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

//
//bool isDefaultConstructor(ClassNode* classNode, MethodNode* methodNode)
//{
//	if(classNode->m_name->m_str == methodNode->m_name->m_str)
//	{
//		if(methodNode->getParameterCount() == methodNode->getDefaultParameterCount())
//		{
//			return true;
//		}
//	}
//	return false;
//}

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
				if (0 != methodNode->m_voidResult || 0 != methodNode->m_resultList)
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
		if (identifyNode->m_str == "array")
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
		|| snt_keyword_class == keyword->m_nodeType);

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

	m_arrayFlag = false;

	ParseConceptList(m_category, m_arrayFlag, conceptList);
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
	std::vector<BaseClassNode*> baseClassNodes;
	m_baseList->collectBaseClassNodes(baseClassNodes);
	count = baseClassNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = baseClassNodes[i]->m_typeName;
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
	std::vector<BaseClassNode*> baseClassNodes;
	m_baseList->collectBaseClassNodes(baseClassNodes);
	count = baseClassNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = baseClassNodes[i]->m_typeName;
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
	std::vector<BaseClassNode*> baseClassNodes;
	m_baseList->collectBaseClassNodes(baseClassNodes);
	std::vector<TypeNode*> baseTypeNodes;
	size_t baseCount = baseClassNodes.size();
	for (size_t i = 0; i < baseCount; ++i)
	{
		TypeNameNode* typeNameNode = baseClassNodes[i]->m_typeName;
		typeNameNode->calcTypeNodes(enclosingTypeNode, templateArguments);
	}
	m_memberList->checkTypeNames(m_typeNode, templateArguments);
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
	std::vector<BaseClassNode*> baseClassNodes;
	m_baseList->collectBaseClassNodes(baseClassNodes);
	std::vector<TypeNode*> baseTypeNodes;
	size_t baseCount = baseClassNodes.size();
	for (size_t i = 0; i < baseCount; ++i)
	{
		TypeNameNode* typeNameNode = baseClassNodes[i]->m_typeName;
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
