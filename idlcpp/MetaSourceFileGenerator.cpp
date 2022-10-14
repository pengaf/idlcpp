#include "MetaSourceFileGenerator.h"
#include "Utility.h"
#include "SourceFile.h"
#include "ProgramNode.h"
#include "NamespaceNode.h"
#include "TokenNode.h"
#include "TokenListNode.h"
#include "IdentifyNode.h"
#include "EnumeratorListNode.h"
#include "EnumeratorNode.h"
#include "EnumNode.h"
#include "ClassNode.h"
#include "DelegateNode.h"
#include "TemplateParametersNode.h"
#include "TemplateClassInstanceNode.h"
#include "TypedefNode.h"
#include "typeDeclarationNode.h"
#include "TypeNameListNode.h"
#include "TypeNameNode.h"
#include "MemberListNode.h"
#include "FieldNode.h"
#include "PropertyNode.h"
#include "MethodNode.h"
#include "ParameterNode.h"
#include "ParameterListNode.h"
#include "AttributeNode.h"
#include "AttributeListNode.h"
#include "TypeTree.h"
#include "Platform.h"
#include "CommonFuncs.h"
#include "Options.h"
#include "Compiler.h"

#include <assert.h>
#include <algorithm>
#include <vector>
#include <string>

static const char* s_defaultTypeCompound = "::pafcore::TypeCompound::none";

const char* typeCompoundToString(TypeCompound typeCompound)
{
	switch (typeCompound)
	{
	case tc_raw_ptr:
		return "::pafcore::TypeCompound::raw_ptr";
	case tc_raw_array:
		return "::pafcore::TypeCompound::raw_array";
	case tc_borrowed_ptr:
		return "::pafcore::TypeCompound::borrowed_ptr";
	case tc_borrowed_array:
		return "::pafcore::TypeCompound::borrowed_array";
	case tc_unique_ptr:
		return "::pafcore::TypeCompound::unique_ptr";
	case tc_unique_array:
		return "::pafcore::TypeCompound::unique_array";
	case tc_shared_ptr:
		return "::pafcore::TypeCompound::shared_ptr";
	case tc_shared_array:
		return "::pafcore::TypeCompound::shared_array";
	default:
		return "::pafcore::TypeCompound::none";
	}
}

const char* parameterPassingToString(ParameterPassing parameterPassing)
{
	switch (parameterPassing)
	{
	case pp_reference:
		return "::pafcore::Passing::reference";
	case pp_const_reference:
		return "::pafcore::Passing::const_reference";
	case pp_rvalue_reference:
		return "::pafcore::Passing::rvalue_reference";
	case pp_const_rvalue_reference:
		return "::pafcore::Passing::const_rvalue_reference";
	default:
		return "::pafcore::Passing::value";
	}
}

void writeMetaConstructor(ClassNode* classNode,
	TemplateArguments* templateArguments,
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation);

void writeMetaPropertyImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<PropertyNode*>& propertyNodes, FILE* file, int indentation);
void writeMetaMethodImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation);
void writeMetaGetSingletonImpls(MemberNode* typeNode, TemplateArguments* templateArguments, FILE* file, int indentation);
void writeEnumMetaConstructor(EnumNode* enumNode, TemplateArguments* templateArguments, std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation);


void MetaSourceFileGenerator::generateCode(FILE* dstFile, SourceFile* sourceFile, const char* fullPathName, const char* baseName)
{
	generateCode_Program(dstFile, sourceFile, fullPathName, baseName);
}

void MetaSourceFileGenerator::generateCode_Program(FILE* file, SourceFile* sourceFile, const char* fileName, const char* cppName)
{
	ProgramNode* programNode = sourceFile->m_syntaxTree;
	char buf[4096];
	std::string pafcorePath;
	GetRelativePath(pafcorePath, fileName, g_options.m_pafcorePath.c_str());
	FormatPathForInclude(pafcorePath);

	writeStringToFile("#pragma once\n\n", file);
	g_compiler.outputUsedTypesForMetaSource(file, sourceFile);
	sprintf_s(buf, "#include \"%s%s\"\n", cppName, g_options.m_metaHeaderFilePostfix.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sAutoRun.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sNameSpace.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sResult.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sArgument.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sInstanceField.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sStaticField.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sInstanceProperty.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sStaticProperty.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sInstanceMethod.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sStaticMethod.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sEnumerator.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sPrimitiveType.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sIterator.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	writeStringToFile("#include <new>\n\n", file);

	writeStringToFile("\nnamespace idlcpp\n{\n\n", file);
	generateCode_Namespace(file, programNode, 1);
	writeStringToFile("}\n\n", file);

	std::vector<TypeNode*> typeNodes;
	CollectTypeNodes(typeNodes, programNode);
	std::reverse(typeNodes.begin(), typeNodes.end());
	size_t typeCount = typeNodes.size();
	for (size_t i = 0; i < typeCount; ++i)
	{
		TypeNode* typeNode = typeNodes[i];
		if (typeNode->getSyntaxNode()->canGenerateMetaCode())
		{
			std::string metaTypeName;
			GetMetaTypeFullName(metaTypeName, typeNode);
			sprintf_s(buf, "AUTO_REGISTER_TYPE(::idlcpp::%s)\n", metaTypeName.c_str());
			writeStringToFile(buf, file);
			//if (typeNode->isTypeDeclaration())
			//{
			//	TypeCategory typeCategory = typeNode->getTypeCategory(0);
			//	const char* typeCategoryName = "";
			//	switch (typeCategory)
			//	{
			//	case enum_type:
			//		typeCategoryName = "enumeration";
			//		break;
			//	case object_type:
			//		typeCategoryName = "object";
			//		break;
			//	case introspectable_type:
			//		typeCategoryName = "introspectable";
			//		break;
			//	case string_type:
			//		typeCategoryName = "string";
			//		break;
			//	case buffer_type:
			//		typeCategoryName = "buffer";
			//		break;
			//	default:
			//		assert(false);
			//	}
			//	std::string typeName;
			//	typeNode->getNativeName(typeName);
			//	sprintf_s(buf, "static_assert(RuntimeTypeOf<%s>::type_category == ::pafcore::MetaCategory::%s, \"type category error\");\n",
			//		typeName.c_str(), typeCategoryName);
			//	writeStringToFile(buf, file);
			//}
		}
	}
}

void MetaSourceFileGenerator::generateCode_Namespace(FILE* file, NamespaceNode* namespaceNode, int indentation)
{
	if (namespaceNode->isNoMeta())
	{
		return;
	}

	std::vector<MemberNode*> memberNodes;
	namespaceNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		switch (memberNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(memberNode), 0, indentation);
			break;
		case snt_class:
			if (!memberNode->isTemplateClass())
			{
				generateCode_Class(file, static_cast<ClassNode*>(memberNode), 0, indentation);
			}
			break;
		case snt_delegate:
			generateCode_Delegate(file, static_cast<DelegateNode*>(memberNode), 0, indentation);
			break;
		case snt_namespace:
			generateCode_Namespace(file, static_cast<NamespaceNode*>(memberNode), indentation);
			break;
		case snt_template_class_instance:
			generateCode_TemplateClassInstance(file, static_cast<TemplateClassInstanceNode*>(memberNode), indentation);
			break;
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(memberNode), 0, indentation);
			break;
		case snt_type_declaration:
			generateCode_TypeDeclaration(file, static_cast<TypeDeclarationNode*>(memberNode), 0, indentation);
			break;
		default:
			assert(false);
		}
	}
}

void MetaSourceFileGenerator::generateCode_Enum(FILE* file, EnumNode* enumNode, TemplateArguments* templateArguments, int indentation)
{
	if (enumNode->isNoMeta())
	{
		return;
	}
	std::vector<EnumeratorNode*> enumeratorNodes;
	enumNode->m_enumeratorList->collectEnumeratorNodes(enumeratorNodes);
	std::sort(enumeratorNodes.begin(), enumeratorNodes.end(), CompareEnumeratorPtr());
	writeEnumMetaConstructor(enumNode, templateArguments, enumeratorNodes, file, indentation);
	writeMetaGetSingletonImpls(enumNode, templateArguments, file, indentation);
}

void MetaSourceFileGenerator::generateCode_TemplateClassInstance(FILE* file, TemplateClassInstanceNode* templateClassInstance, int indentation)
{
	if (templateClassInstance->isNoMeta())
	{
		return;
	}
	ClassNode* classNode = static_cast<ClassNode*>(templateClassInstance->m_classTypeNode->m_classNode);
	generateCode_Class(file, classNode, templateClassInstance, indentation);
}

void writeOverrideFunction(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);
	std::string className;
	classNode->getNativeName(className, templateArguments);

	sprintf_s(buf, "bool %s::destruct(void* address)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return DestructorCaller<%s>::Call(address);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "bool %s::copyConstruct(void* dst, const void* src)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return CopyConstructorCaller<%s>::Call(dst, src);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "bool %s::copyAssign(void* dst, const void* src)\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "return CopyAssignmentCaller<%s>::Call(dst, src);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	if (classNode->needSubclassProxy(templateArguments))
	{
		std::string subclassProxyName;
		GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);
		sprintf_s(buf, "void* %s::createSubclassProxy(::pafcore::SubclassInvoker* subclassInvoker)\n", metaClassName.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n", file, indentation);
		sprintf_s(buf, "return new %s(subclassInvoker);\n", subclassProxyName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("}\n\n", file, indentation);

		sprintf_s(buf, "void %s::destroySubclassProxy(void* subclassProxy)\n", metaClassName.c_str());
		writeStringToFile(buf, file, indentation);
		writeStringToFile("{\n", file, indentation);

		sprintf_s(buf, "delete reinterpret_cast<%s*>(subclassProxy)->m_subclassInvoker;\n", subclassProxyName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "reinterpret_cast<%s*>(subclassProxy)->m_subclassInvoker = 0;\n", subclassProxyName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("delete subclassProxy;\n", file, indentation + 1);
		writeStringToFile("}\n\n", file, indentation);
	}
}


struct CompareMethodNode
{
	bool operator()(const MethodNode* m1, const MethodNode* m2) const
	{
		int cmp = m1->m_name->m_str.compare(m2->m_name->m_str);
		if (0 != cmp)
		{
			return cmp < 0;
		}
		return m1->getParameterCount() < m2->getParameterCount();
	}
};

void MetaSourceFileGenerator::generateCode_Delegate(FILE* file, DelegateNode* delegateNode, TemplateArguments* templateArguments, int indentation)
{
	if (delegateNode->isNoMeta())
	{
		return;
	}
	generateCode_Class(file, delegateNode->m_classNode, 0, indentation);
}

void MetaSourceFileGenerator::generateCode_Class(FILE* file, ClassNode* classNode, TemplateClassInstanceNode* templateClassInstance, int indentation)
{
	if (classNode->isNoMeta())
	{
		return;
	}
	TemplateArguments* templateArguments = templateClassInstance ? &templateClassInstance->m_templateArguments : 0;

	std::vector<IdentifyNode*> reservedNames;
	std::vector<TokenNode*> reservedOperators;
	if (templateClassInstance && templateClassInstance->m_tokenList
		&& templateClassInstance->m_classTypeNode->m_classNode == classNode)
	{
		assert(classNode->m_typeNode == templateClassInstance->m_classTypeNode);
		templateClassInstance->getReservedMembers(reservedNames, reservedOperators);
	}
	bool hasReservedMember = (!reservedNames.empty() || !reservedOperators.empty());

	std::vector<MemberNode*> memberNodes;
	std::vector<MethodNode*> methodNodes;
	std::vector<MethodNode*> staticMethodNodes;
	std::vector<PropertyNode*> propertyNodes;
	std::vector<PropertyNode*> staticPropertyNodes;
	std::vector<FieldNode*> fieldNodes;
	std::vector<FieldNode*> staticFieldNodes;
	std::vector<MemberNode*> nestedTypeNodes;
	std::vector<MemberNode*> nestedTypeAliasNodes;

	classNode->m_memberList->collectMemberNodes(memberNodes);
	size_t memberCount = memberNodes.size();
	for (size_t i = 0; i < memberCount; ++i)
	{
		MemberNode* memberNode = memberNodes[i];
		if (hasReservedMember)
		{
			if (snt_method == memberNode->m_nodeType || snt_property == memberNode->m_nodeType)
			{
				if (!std::binary_search(reservedNames.begin(), reservedNames.end(), memberNode->m_name, CompareIdentifyPtr()))
				{
					continue;
				}
			}
		}
		if (!memberNode->isNoMeta())
		{
			if (snt_method == memberNode->m_nodeType)
			{
				MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
				if (memberNode->m_name->m_str != classNode->m_name->m_str)
				{
					if (methodNode->isStatic())
					{
						staticMethodNodes.push_back(methodNode);
					}
					else
					{
						methodNodes.push_back(methodNode);
					}
				}
			}
			else if (snt_property == memberNode->m_nodeType)
			{
				PropertyNode* propertyNode = static_cast<PropertyNode*>(memberNode);
				if (propertyNode->isStatic())
				{
					propertyNode->m_orderIndex = staticPropertyNodes.size();
					staticPropertyNodes.push_back(propertyNode);
				}
				else
				{
					propertyNode->m_orderIndex = propertyNodes.size();
					propertyNodes.push_back(propertyNode);
				}
			}
			else if (snt_field == memberNode->m_nodeType)
			{
				FieldNode* fieldNode = static_cast<FieldNode*>(memberNode);
				if (fieldNode->isStatic())
				{
					fieldNode->m_orderIndex = staticFieldNodes.size();
					staticFieldNodes.push_back(fieldNode);
				}
				else
				{
					fieldNode->m_orderIndex = fieldNodes.size();
					fieldNodes.push_back(fieldNode);
				}
			}
			else if (snt_enum == memberNode->m_nodeType ||
				snt_delegate == memberNode->m_nodeType ||
				snt_class == memberNode->m_nodeType)
			{
				nestedTypeNodes.push_back(memberNode);
			}
			else if (snt_typedef == memberNode->m_nodeType ||
				snt_type_declaration == memberNode->m_nodeType)
			{
				nestedTypeAliasNodes.push_back(memberNode);
			}
			else
			{
				assert(false);
			}
		}
	}

	//if(!classNode->isAbstractClass())
	{
		auto it = classNode->m_additionalMethods.begin();
		auto end = classNode->m_additionalMethods.end();
		for (; it != end; ++it)
		{
			MethodNode* methodNode = *it;
			if (!reservedNames.empty())
			{
				if (!std::binary_search(reservedNames.begin(), reservedNames.end(), methodNode->m_name, CompareIdentifyPtr()))
				{
					continue;
				}
			}
			if (!methodNode->isNoMeta())
			{
				staticMethodNodes.push_back(methodNode);
			}
		}
	}


	writeMetaConstructor(classNode, templateArguments, nestedTypeNodes, nestedTypeAliasNodes,
		staticFieldNodes, staticPropertyNodes, staticMethodNodes,
		fieldNodes, propertyNodes, methodNodes, file, indentation);

	writeOverrideFunction(classNode, templateArguments, file, indentation);
	writeMetaPropertyImpls(classNode, templateArguments, propertyNodes, file, indentation);
	writeMetaPropertyImpls(classNode, templateArguments, staticPropertyNodes, file, indentation);
	writeMetaMethodImpls(classNode, templateArguments, methodNodes, false, file, indentation);
	writeMetaMethodImpls(classNode, templateArguments, staticMethodNodes, true, file, indentation);
	writeMetaGetSingletonImpls(classNode, templateArguments, file, indentation);

	if (classNode->needSubclassProxy(templateArguments))
	{
		generateCode_SubclassProxy(file, classNode, templateArguments, indentation);
	}

	size_t subTypeCount = nestedTypeNodes.size();
	for (size_t i = 0; i < subTypeCount; ++i)
	{
		MemberNode* typeNode = nestedTypeNodes[i];
		switch (typeNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(typeNode), templateArguments, indentation);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(typeNode), templateClassInstance, indentation);
			break;
		case snt_delegate:
			generateCode_Delegate(file, static_cast<DelegateNode*>(typeNode), templateArguments, indentation);
			break;
		default:
			assert(false);
		}
	}

	size_t typeAliasCount = nestedTypeAliasNodes.size();
	for (size_t i = 0; i < typeAliasCount; ++i)
	{
		MemberNode* typeAliasNode = nestedTypeAliasNodes[i];
		switch (typeAliasNode->m_nodeType)
		{
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(typeAliasNode), templateArguments, indentation);
			break;
		case snt_type_declaration:
			generateCode_TypeDeclaration(file, static_cast<TypeDeclarationNode*>(typeAliasNode), templateArguments, indentation);
			break;
		default:
			assert(false);
		}
	}
}

const char* g_metaPropertyImplPrefix = "::pafcore::ErrorCode ";

const char* g_metaSimplePropertyGetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Variant* value)\n";
const char* g_metaSimplePropertySetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Variant* value)\n";

const char* g_metaArrayPropertyGetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, size_t index, ::pafcore::Variant* value)\n";
const char* g_metaArrayPropertySetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, size_t index, ::pafcore::Variant* value)\n";
const char* g_metaArrayPropertySizeImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Variant* size)\n";

const char* g_metaCollectionPropertyGetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Iterator* iterator, ::pafcore::Variant* value)\n";
const char* g_metaCollectionPropertySetImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Iterator* iterator, size_t replacingCount, ::pafcore::Variant* value, size_t count)\n";
const char* g_metaCollectionPropertyIterateImplPostfix = "(::pafcore::InstanceProperty* instanceProperty, ::pafcore::Variant* that, ::pafcore::Variant* iterator)\n";

const char* g_metaStaticSimplePropertyGetImplPostfix = "(::pafcore::Variant* value)\n";
const char* g_metaStaticSimplePropertySetImplPostfix = "(::pafcore::Variant* value)\n";

const char* g_metaStaticArrayPropertyGetImplPostfix = "(size_t index, ::pafcore::Variant* value)\n";
const char* g_metaStaticArrayPropertySetImplPostfix = "(size_t index, ::pafcore::Variant* value)\n";
const char* g_metaStaticArrayPropertySizeImplPostfix = "(::pafcore::Variant* size)\n";

const char* g_metaStaticCollectionPropertyGetImplPostfix = "(::pafcore::Iterator* iterator, ::pafcore::Variant* value)\n";
const char* g_metaStaticCollectionPropertySetImplPostfix = "(::pafcore::Iterator* iterator, size_t replacingCount, ::pafcore::Variant* value, size_t count)\n";
const char* g_metaStaticCollectionPropertyIterateImplPostfix = "(::pafcore::Variant* iterator)\n";



void writeMetaPropertyGetImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_get_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertyGetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertyGetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertyGetImplPostfix, file);
			break;
		}
	}
	else
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertyGetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertyGetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertyGetImplPostfix, file);
			break;
		}
	}
	writeStringToFile("{\n", file, indentation);
	TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);

	if (!propertyNode->isStatic())
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::pafcore::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}

	switch (propertyNode->m_get->m_typeCompound)
	{
	case tc_raw_ptr:
		writeStringToFile("value->assignRawPtr(", file, indentation + 1);
		break;
	case tc_raw_array:
		writeStringToFile("value->assignRawArray(", file, indentation + 1);
		break;
	case tc_borrowed_ptr:
		writeStringToFile("value->assignBorrowedPtr(", file, indentation + 1);
		break;
	case tc_borrowed_array:
		writeStringToFile("value->assignBorrowedArray(", file, indentation + 1);
		break;
	case tc_unique_ptr:
		writeStringToFile("value->assignUniquePtr(", file, indentation + 1);
		break;
	case tc_unique_array:
		writeStringToFile("value->assignUniqueArray(", file, indentation + 1);
		break;
	case tc_shared_ptr:
		writeStringToFile("value->assignSharedPtr(", file, indentation + 1);
		break;
	case tc_shared_array:
		writeStringToFile("value->assignSharedArray(", file, indentation + 1);
		break;
	default:
		writeStringToFile("value->assignValue(", file, indentation + 1);
	}

	const char* strIndex = "";
	if (propertyNode->isArray())
	{
		strIndex = "index";
	}
	else if (propertyNode->isCollection())
	{
		strIndex = "iterator";
	}

	if (propertyNode->isStatic())
	{
		if (propertyNode->m_get->m_nativeName)
		{
			sprintf_s(buf, "%s(%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(), strIndex);
		}
		else
		{
			sprintf_s(buf, "%s::get_%s(%s));\n", className.c_str(), propertyNode->m_name->m_str.c_str(), strIndex);
		}		
		writeStringToFile(buf, file);
	}
	else
	{
		if (propertyNode->m_get->m_nativeName)
		{
			if (propertyNode->m_get->m_nativeName->m_str.find(':') != std::string::npos)
			{
				sprintf_s(buf, "%s(self%s%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(),
					propertyNode->isSimple() ? "" : ", ", strIndex);
			}
			else
			{
				sprintf_s(buf, "self->%s(%s));\n", propertyNode->m_get->m_nativeName->m_str.c_str(), strIndex);
			}
		}
		else
		{
			sprintf_s(buf, "self->get_%s(%s));\n", propertyNode->m_name->m_str.c_str(), strIndex);
		}
		writeStringToFile(buf, file);
	}
	writeStringToFile("return ::pafcore::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertySetImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_set_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertySetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertySetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertySetImplPostfix, file);
			break;
		}
	}
	else
	{
		switch (propertyNode->getCategory())
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertySetImplPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertySetImplPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertySetImplPostfix, file);
			break;
		}

	}
	writeStringToFile("{\n", file, indentation);

	TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);

	if (!propertyNode->isStatic())
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::pafcore::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}

	const char* argDeference = "";
	if (tc_none == propertyNode->m_set->m_typeCompound)
	{
		if( (primitive_type == typeCategory || enum_type == typeCategory) && 
			(pp_value == propertyNode->m_set->m_passing || pp_const_reference == propertyNode->m_set->m_passing || pp_const_rvalue_reference == propertyNode->m_set->m_passing))
		{
			if (primitive_type == typeCategory)
			{
				sprintf_s(buf, "%s arg;\n", typeName.c_str());
				writeStringToFile(buf, file, indentation + 1);
				sprintf_s(buf, "if(!value->castToPrimitive(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &arg))\n", typeName.c_str());
				writeStringToFile(buf, file, indentation + 1);
			}
			else if (enum_type == typeCategory)
			{
				sprintf_s(buf, "%s arg;\n", typeName.c_str());
				writeStringToFile(buf, file, indentation + 1);
				sprintf_s(buf, "if(!value->castToEnum(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &arg))\n", typeName.c_str());
				writeStringToFile(buf, file, indentation + 1);
			}
		}
		else
		{
			sprintf_s(buf, "%s* arg;\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(!value->castToRawPointer(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&arg))\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			argDeference = "*";
		}
	}
	else if (tc_raw_ptr == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::RawPtr<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToRawPtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_raw_array == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::RawArray<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToRawArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_borrowed_ptr == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::BorrowedPtr<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToBorrowedPtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_borrowed_array == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::BorrowedArray<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToBorrowedArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_unique_ptr == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::UniquePtr<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToUniquePtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_unique_array == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::UniqueArray<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToUniqueArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_ptr == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::SharedPtr<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToSharedPtr<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_array == propertyNode->m_set->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::SharedArray<%s> arg;\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!value->castToSharedArray<%s>(arg))\n", typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("{\n", file, indentation + 1);
	writeStringToFile("return ::pafcore::ErrorCode::e_invalid_property_type;\n", file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);

	if (propertyNode->isStatic())
	{
		if (propertyNode->m_set->m_nativeName)
		{
			sprintf_s(buf, "%s(", propertyNode->m_set->m_nativeName->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "%s::set_%s(", className.c_str(), propertyNode->m_name->m_str.c_str());
		}
	}
	else
	{
		if (propertyNode->m_set->m_nativeName)
		{
			if (propertyNode->m_set->m_nativeName->m_str.find(':') != std::string::npos)
			{
				sprintf_s(buf, "%s(self, ", propertyNode->m_set->m_nativeName->m_str.c_str());
			}
			else
			{
				sprintf_s(buf, "self->%s(", propertyNode->m_set->m_nativeName->m_str.c_str());
			}
		}
		else
		{
			sprintf_s(buf, "self->set_%s(", propertyNode->m_name->m_str.c_str());
		}
	}
	writeStringToFile(buf, file, indentation + 1);

	const char* otherArgs = "";
	if (propertyNode->isArray())
	{
		otherArgs = "index, ";
	}
	else if (propertyNode->isCollection())
	{
		otherArgs = "iterator, removeCount, ";
	}

	if (!propertyNode->isSimple())
	{
		writeStringToFile("index, ", file);
	}

	bool needMove = (pp_rvalue_reference == propertyNode->m_set->m_passing || pp_const_rvalue_reference == propertyNode->m_set->m_passing);

	sprintf_s(buf, "%s%s%sarg%s);\n", otherArgs, needMove ? "std::move(" : "", argDeference, needMove ? ")" : "");

	writeStringToFile(buf, file, 0);
	writeStringToFile("return ::pafcore::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertySizeImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_size_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		writeStringToFile(g_metaStaticArrayPropertySizeImplPostfix, file);
	}
	else
	{
		writeStringToFile(g_metaArrayPropertySizeImplPostfix, file);
	}
	writeStringToFile("{\n", file, indentation);

	if (propertyNode->isStatic())
	{
		sprintf_s(buf, "size = %s::size_%s();\n", className.c_str(), propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::pafcore::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
		
		sprintf_s(buf, "size = self->size_%s();\n", propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("return ::pafcore::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertyIteratorImpl(ClassNode* classNode, TemplateArguments* templateArguments, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string className;
	std::string metaClassName;

	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaPropertyImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_iterate_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	if (propertyNode->isStatic())
	{
		writeStringToFile(g_metaStaticCollectionPropertyIterateImplPostfix, file);
	}
	else
	{
		writeStringToFile(g_metaCollectionPropertyIterateImplPostfix, file);
	}
	writeStringToFile("{\n", file, indentation);

	if (propertyNode->isStatic())
	{
		sprintf_s(buf, "iterator = %s::iterate_%s();\n", className.c_str(), propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	else
	{
		sprintf_s(buf, "%s* self;\n", className.c_str());
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("if(!that->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::pafcore::ErrorCode::e_invalid_this_type;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);

		sprintf_s(buf, "iterator = self->iterate_%s();\n", propertyNode->m_name->m_str.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("return ::pafcore::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaPropertyImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<PropertyNode*>& propertyNodes, FILE* file, int indentation)
{
	size_t propertyCount = propertyNodes.size();
	if (0 < propertyCount)
	{
		for (size_t i = 0; i < propertyCount; ++i)
		{
			PropertyNode* propertyNode = propertyNodes[i];
			if (0 != propertyNode->m_get)
			{
				writeMetaPropertyGetImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			if (0 != propertyNode->m_set)
			{
				writeMetaPropertySetImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			if (propertyNode->isArray())
			{
				writeMetaPropertySizeImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
			else if (propertyNode->isCollection())
			{
				writeMetaPropertyIteratorImpl(classNode, templateArguments, propertyNodes[i], file, indentation);
			}
		}
	}
}

const char g_metaMethodImplPrefix[] = "::pafcore::ErrorCode ";
const char g_metaMethodImplPostfix[] = "(::pafcore::Variant* result, ::pafcore::Variant** args, uint32_t numArgs)\n";

void writeMetaMethodImpl_CastSelf(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getNativeName(className, templateArguments);
	sprintf_s(buf, "%s* self;\n", className.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("if(!args[0]->castToRawPointer(GetSingleton(), (void**)&self))\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("return ::pafcore::ErrorCode::e_invalid_this_type;\n", file, indentation + 1);
	writeStringToFile("}\n", file, indentation);
}

void writeMetaMethodImpl_CastParam(ClassNode* classNode, TemplateArguments* templateArguments, ParameterNode* parameterNode, size_t argIndex, size_t paramIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);

	const char* argDeference = "";
	if (tc_none == parameterNode->m_typeCompound)
	{
		if ((primitive_type == typeCategory || enum_type == typeCategory) &&
			(pp_value == parameterNode->m_passing || pp_const_reference == parameterNode->m_passing || pp_const_rvalue_reference == parameterNode->m_passing))
		{
			if (primitive_type == typeCategory)
			{
				sprintf_s(buf, "%s a%zd;\n", typeName.c_str(), paramIndex);
				writeStringToFile(buf, file, indentation);
				sprintf_s(buf, "if(!args[%zd]->castToPrimitive(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &a%zd))\n", argIndex, typeName.c_str(), paramIndex);
				writeStringToFile(buf, file, indentation + 1);
			}
			else if (enum_type == typeCategory)
			{
				sprintf_s(buf, "%s a%zd;\n", typeName.c_str(), paramIndex);
				writeStringToFile(buf, file, indentation);
				sprintf_s(buf, "if(!args[%zd]->castToEnum(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), &a%zd))\n", argIndex, typeName.c_str(), paramIndex);
				writeStringToFile(buf, file, indentation + 1);
			}
		}
		else
		{
			sprintf_s(buf, "%s* a%zd;\n", typeName.c_str(), paramIndex);
			writeStringToFile(buf, file, indentation + 1);
			sprintf_s(buf, "if(!args[%zd]->castToRawPointer(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)&a%zd))\n", argIndex, typeName.c_str(), paramIndex);
			writeStringToFile(buf, file, indentation + 1);
			argDeference = "*";
		}
	}
	else if (tc_raw_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::RawPtr<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToRawPtr<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_raw_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::RawArray<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToRawArray<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_borrowed_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::BorrowedPtr<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToBorrowedPtr<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_borrowed_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::BorrowedArray<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToBorrowedArray<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_unique_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::UniquePtr<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToUniquePtr<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_unique_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::UniqueArray<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToUniqueArray<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_ptr == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::SharedPtr<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToSharedPtr<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	else if (tc_shared_array == parameterNode->m_typeCompound)
	{
		sprintf_s(buf, "::pafcore::SharedArray<%s> a%zd;\n", typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
		sprintf_s(buf, "if(!args[%zd]->castToSharedArray<%s>(a%zd))\n", argIndex, typeName.c_str(), paramIndex);
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "return ::pafcore::ErrorCode::e_invalid_arg_type_%zd;\n", paramIndex + 1);
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);
}

void writeMetaMethodImpl_UseParam(ClassNode* classNode, TemplateArguments* templateArguments, ParameterNode* parameterNode, size_t paramIndex, FILE* file)
{
	char buf[126];
	char strArg[64];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);

	if (tc_none == parameterNode->m_typeCompound)
	{
		if ((primitive_type == typeCategory || enum_type == typeCategory) &&
			(pp_value == parameterNode->m_passing || pp_const_reference == parameterNode->m_passing || pp_const_rvalue_reference == parameterNode->m_passing))
		{
			sprintf_s(strArg, "a%zd", paramIndex);
		}
		else
		{
			sprintf_s(strArg, "*a%zd", paramIndex);
		}
	}
	else
	{
		sprintf_s(strArg, "a%zd", paramIndex);
	}
	if (0 != paramIndex)
	{
		writeStringToFile(", ", file, 0);
	}
	bool needMove = (pp_rvalue_reference == parameterNode->m_passing || pp_const_rvalue_reference == parameterNode->m_passing);
	if (needMove)
	{
		sprintf_s(buf, "std::move(%s)", strArg);
		writeStringToFile(buf, file, 0);
	}
	else
	{
		writeStringToFile(strArg, file, 0);
	}
}

void writeMetaMethodImpl_Call(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, std::vector<ParameterNode*>& parameterNodes, size_t usedParamCount, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getNativeName(className, templateArguments);

	IdentifyNode* methodNameNode = methodNode->m_nativeName ? methodNode->m_nativeName : methodNode->m_name;

	//size_t paramCount = parameterNodes.size();
	sprintf_s(buf, "if(%zd == numArgs)\n", usedParamCount + (isStatic ? 0 : 1));
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	if (methodNode->m_resultTypeName)
	{
		switch (methodNode->m_resultTypeCompound)
		{
		case tc_raw_ptr:
			writeStringToFile("result->assignRawPtr(", file, indentation + 1);
			break;
		case tc_raw_array:
			writeStringToFile("result->assignRawArray(", file, indentation + 1);
			break;
		case tc_borrowed_ptr:
			writeStringToFile("result->assignBorrowedPtr(", file, indentation + 1);
			break;
		case tc_borrowed_array:
			writeStringToFile("result->assignBorrowedArray(", file, indentation + 1);
			break;
		case tc_unique_ptr:
			writeStringToFile("result->assignUniquePtr(", file, indentation + 1);
			break;
		case tc_unique_array:
			writeStringToFile("result->assignUniqueArray(", file, indentation + 1);
			break;
		case tc_shared_ptr:
			writeStringToFile("result->assignSharedPtr(", file, indentation + 1);
			break;
		case tc_shared_array:
			writeStringToFile("result->assignSharedArray(", file, indentation + 1);
			break;
		default:
			writeStringToFile("result->assignValue(", file, indentation + 1);
		}
	}
	else
	{
		writeStringToFile("", file, indentation + 1);
	}


	if (methodNode->isStatic())
	{
		if (methodNode->m_nativeName)
		{
			sprintf_s(buf, "%s(", methodNameNode->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "%s::%s(", className.c_str(), methodNameNode->m_str.c_str());
		}
	}
	else
	{
		if (methodNameNode->m_str.find(':') != std::string::npos)
		{
			sprintf_s(buf, "%s(self%s", methodNameNode->m_str.c_str(), 0 == usedParamCount ? "" : ", ");
		}
		else
		{
			sprintf_s(buf, "self->%s(", methodNameNode->m_str.c_str());
		}
	}
	writeStringToFile(buf, file, 0);
	for (size_t i = 0; i < usedParamCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		writeMetaMethodImpl_UseParam(classNode, templateArguments, parameterNode, i, file);
	}
	if (methodNode->m_resultTypeName)
	{
		writeStringToFile(")", file, 0);
	}
	writeStringToFile(");\n", file, 0);

	writeStringToFile("return ::pafcore::ErrorCode::s_ok;\n", file, indentation + 1);
	writeStringToFile("}\n", file, indentation);
}

void writeMetaMethodImpl(
	ClassNode* classNode, 
	TemplateArguments* templateArguments,
	MethodNode* methodNode,
	bool isStatic, 
	size_t methodIndex, 
	FILE* file, 
	int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	writeStringToFile(g_metaMethodImplPrefix, file, indentation);
	sprintf_s(buf, "%s::%s_%s", metaClassName.c_str(),
		classNode->m_name->m_str.c_str(), methodNode->m_name->m_str.c_str());
	writeStringToFile(buf, file);
	writeStringToFile(g_metaMethodImplPostfix, file);
	writeStringToFile("{\n", file, indentation);

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t paramCount = parameterNodes.size();
	size_t firstDefaultParam = methodNode->getFirstDefaultParameter();

	size_t minParamCount = firstDefaultParam < paramCount ? firstDefaultParam : paramCount;
	size_t minArgCount = minParamCount + (isStatic ? 0 : 1);
	if (0 < minArgCount)
	{
		sprintf_s(buf, "if(numArgs < %zd)\n", minArgCount);
		writeStringToFile(buf, file, indentation + 1);
		writeStringToFile("{\n", file, indentation + 1);
		writeStringToFile("return ::pafcore::ErrorCode::e_invalid_too_few_arguments;\n", file, indentation + 2);
		writeStringToFile("}\n", file, indentation + 1);
	}
	if (!isStatic)
	{
		writeMetaMethodImpl_CastSelf(classNode, templateArguments, methodNode, file, indentation + 1);
	}
	for (size_t i = 0; i < minParamCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		size_t argIndex = isStatic ? i : i + 1;
		writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
	}
	for (size_t i = minParamCount; i < paramCount; ++i)
	{
		writeMetaMethodImpl_Call(classNode, templateArguments, methodNode, parameterNodes, i, isStatic, file, indentation + 1);
		ParameterNode* parameterNode = parameterNodes[i];
		size_t argIndex = isStatic ? i : i + 1;
		writeMetaMethodImpl_CastParam(classNode, templateArguments, parameterNode, argIndex, i, file, indentation);
	}
	writeMetaMethodImpl_Call(classNode, templateArguments, methodNode, parameterNodes, paramCount, isStatic, file, indentation + 1);

	writeStringToFile("return ::pafcore::ErrorCode::e_invalid_too_many_arguments;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaMethodImpls(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation)
{
	size_t methodCount = methodNodes.size();
	if (0 < methodCount)
	{
		for (size_t i = 0; i < methodCount; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaMethodImpl(classNode, templateArguments, methodNode, isStatic, i, file, indentation);
		}
	}
}

void writeMetaConstructor_attributesForType(std::map<SyntaxNodeImpl*, size_t> attributesOffsets, SyntaxNodeImpl* memberNode, FILE* file, int indentation)
{
	char buf[4096];
	auto it = attributesOffsets.find(memberNode);
	if (it != attributesOffsets.end())
	{
		sprintf_s(buf, "m_attributes = &s_attributeses[%zd];\n", it->second);
		writeStringToFile(buf, file, indentation);
	}
}

struct AttributeOffsetAndCount
{
	SyntaxNodeImpl* node;
	size_t offset;
	size_t count;
};

template<typename T>
void writeMetaConstructor_Attributes_Member(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	T* memberNode, FILE* file, int indentation)
{
	AttributeListNode* attributeList = memberNode->m_attributeList;
	if (attributeList)
	{
		std::vector<AttributeNode*> attributeNodes;
		attributeList->collectAttributeNodes(attributeNodes);
		std::sort(attributeNodes.begin(), attributeNodes.end(), CompareAttributePtr());
		size_t count = attributeNodes.size();
		for (size_t i = 0; i < count; ++i)
		{
			AttributeNode* attributeNode = attributeNodes[i];
			writeStringToFile("{ \"", file, indentation);
			writeStringToFile(attributeNode->m_name->m_str.c_str(), file);
			if (attributeNode->m_u8content)
			{
				writeStringToFile("\", u8\"", file);
			}
			else
			{
				writeStringToFile("\", \"", file);
			}
			if (attributeNode->m_content)
			{
				writeStringToFile(attributeNode->m_content->m_str.c_str(), file);
			}
			writeStringToFile("\" },\n", file);
		}
		AttributeOffsetAndCount aoac;
		aoac.node = memberNode;
		aoac.offset = offset;
		aoac.count = count;
		attributeOffsetAndCounts.push_back(aoac);
		offset += count;
	}
}

void writeMetaConstructor_Attributes_Method(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<MethodNode*>::iterator first, std::vector<MethodNode*>::iterator last, FILE* file, int indentation)
{
	std::vector<AttributeNode*> attributeNodes;
	for (auto it = first; it != last; ++it)
	{
		MethodNode* methodNode = *it;
		AttributeListNode* attributeList = methodNode->m_attributeList;
		if (attributeList)
		{
			attributeList->collectAttributeNodes(attributeNodes);
		}
	}
	if (!attributeNodes.empty())
	{
		std::sort(attributeNodes.begin(), attributeNodes.end(), CompareAttributePtr());
		size_t count = attributeNodes.size();
		for (size_t i = 0; i < count; ++i)
		{
			AttributeNode* attributeNode = attributeNodes[i];
			writeStringToFile("{ \"", file, indentation);
			writeStringToFile(attributeNode->m_name->m_str.c_str(), file);
			writeStringToFile("\", u8\"", file);
			if (attributeNode->m_content)
			{
				writeStringToFile(attributeNode->m_content->m_str.c_str(), file);
			}
			writeStringToFile("\" },\n", file);
		}
		AttributeOffsetAndCount aoac;
		aoac.node = *first;
		aoac.offset = offset;
		aoac.count = count;
		attributeOffsetAndCounts.push_back(aoac);
		offset += count;
	}
}

template<typename T>
void writeMetaConstructor_Attributes_Members(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<T*>& memberNodes, FILE* file, int indentation)
{
	size_t count = memberNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		T* node = memberNodes[i];
		writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, node, file, indentation);
	}
}

void writeMetaConstructor_Attributes_Methods(std::vector<AttributeOffsetAndCount>& attributeOffsetAndCounts, size_t& offset,
	std::vector<MethodNode*>& methodNodes, FILE* file, int indentation)
{
	std::vector<MethodNode*>::iterator begin = methodNodes.begin();
	std::vector<MethodNode*>::iterator end = methodNodes.end();
	std::vector<MethodNode*>::iterator first = begin;
	std::vector<MethodNode*>::iterator last = begin;
	for (; first != end;)
	{
		++last;
		if (last == end || (*last)->m_name->m_str != (*first)->m_name->m_str)
		{
			writeMetaConstructor_Attributes_Method(attributeOffsetAndCounts, offset, first, last, file, indentation);
			first = last;
		}
	}
}

template<typename T>
bool noAttributes(T& t)
{
	size_t count = t.size();
	for (size_t i = 0; i < count; ++i)
	{
		AttributeListNode* node = t[i]->m_attributeList;
		if (node)
		{
			return false;
		}
	}
	return true;
}


void writeMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	ClassNode* classNode,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	if (0 == classNode->m_attributeList
		&& noAttributes(staticFieldNodes)
		&& noAttributes(staticPropertyNodes)
		&& noAttributes(staticMethodNodes)
		&& noAttributes(fieldNodes)
		&& noAttributes(propertyNodes)
		&& noAttributes(methodNodes))
	{
		return;
	}

	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;

	writeStringToFile("static ::pafcore::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, classNode, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, staticFieldNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, staticPropertyNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Methods(attributeOffsetAndCounts, offset, staticMethodNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, fieldNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, propertyNodes, file, indentation + 1);
	writeMetaConstructor_Attributes_Methods(attributeOffsetAndCounts, offset, methodNodes, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::pafcore::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeEnumMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	EnumNode* enumNode, std::vector<EnumeratorNode*>& enumeratorNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	if (0 == enumNode->m_attributeList
		&& noAttributes(enumeratorNodes))
	{
		return;
	}
	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;
	writeStringToFile("static ::pafcore::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, enumNode, file, indentation + 1);
	writeMetaConstructor_Attributes_Members(attributeOffsetAndCounts, offset, enumeratorNodes, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::pafcore::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeTypeDefMetaConstructor_Attributeses(std::map<SyntaxNodeImpl*, size_t>& attributessOffsets,
	MemberNode* memberNode, FILE* file, int indentation)
{
	char buf[4096];
	if (0 == memberNode->m_attributeList)
	{
		return;
	}
	size_t offset = 0;
	std::vector<AttributeOffsetAndCount> attributeOffsetAndCounts;
	writeStringToFile("static ::pafcore::Attribute s_attributes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeMetaConstructor_Attributes_Member(attributeOffsetAndCounts, offset, memberNode, file, indentation + 1);
	writeStringToFile("};\n", file, indentation);

	size_t size = attributeOffsetAndCounts.size();

	writeStringToFile("static ::pafcore::Attributes s_attributeses[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < size; ++i)
	{
		AttributeOffsetAndCount& aoac = attributeOffsetAndCounts[i];
		sprintf_s(buf, "{ %zd, &s_attributes[%zd] },\n", aoac.count, aoac.offset);
		writeStringToFile(buf, file, indentation + 1);
		attributessOffsets.insert(std::make_pair(aoac.node, i));
	}
	writeStringToFile("};\n", file, indentation);
}

void writeMetaConstructor_Fields(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t>& attributesOffsets,
	std::vector<FieldNode*>& fieldNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	if (fieldNodes.empty())
	{
		return;
	}
	std::string className;
	classNode->getNativeName(className, templateArguments);
	size_t count = fieldNodes.size();

	if (isStatic)
	{
		writeStringToFile("static ::pafcore::StaticField s_staticFields[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::pafcore::InstanceField s_instanceFields[] = \n", file, indentation);
	}
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		FieldNode* fieldNode = fieldNodes[i];

		if (fieldNode->m_attributeList)
		{
			auto it = attributesOffsets.find(fieldNode);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		IdentifyNode* fieldNameNode = fieldNode->m_nativeName ? fieldNode->m_nativeName : fieldNode->m_name;
		char arraySize[512];
		if (fieldNode->isArray())
		{
			sprintf_s(arraySize, "paf_field_array_size_of(%s, %s)", className.c_str(), fieldNameNode->m_str.c_str());
		}
		else
		{
			strcpy_s(arraySize, "0");
		}
		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, fieldNode->m_typeName, templateArguments);
		const char* typeCompound = typeCompoundToString(fieldNode->m_typeCompound);

		if (isStatic)
		{
			sprintf_s(buf, "::pafcore::StaticField(\"%s\", %s, RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s, (size_t)&%s::%s),\n",
				fieldNode->m_name->m_str.c_str(), strAttributes, typeName.c_str(), typeCompound, arraySize, className.c_str(), fieldNameNode->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "::pafcore::InstanceField(\"%s\", %s, GetSingleton(), RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s, offsetof(%s, %s)),\n",
				fieldNode->m_name->m_str.c_str(), strAttributes, typeName.c_str(), typeCompound, arraySize, className.c_str(), fieldNameNode->m_str.c_str());
		}

		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticFields = s_staticFields;\n", file, indentation);
		writeStringToFile("m_staticFieldCount = paf_array_size_of(s_staticFields);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceFields = s_instanceFields;\n", file, indentation);
		writeStringToFile("m_instanceFieldCount = paf_array_size_of(s_instanceFields);\n", file, indentation);
	}
}

void writeMetaConstructor_Properties(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<PropertyNode*>& propertyNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	char getterFunc[256];
	char setterFunc[256];
	char valueType[256];

	if (propertyNodes.empty())
	{
		return;
	}
	size_t count = propertyNodes.size();

	if (isStatic)
	{
		writeStringToFile("static ::pafcore::StaticProperty s_staticProperties[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::pafcore::InstanceProperty s_instanceProperties[] = \n", file, indentation);
	}

	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		PropertyNode* propertyNode = propertyNodes[i];
		if (propertyNode->m_attributeList)
		{
			auto it = attributesOffsets.find(propertyNode);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, propertyNode->m_typeName, templateArguments);
		sprintf_s(valueType, "RuntimeTypeOf<%s>::RuntimeType::GetSingleton()", typeName.c_str());

		const char* getterTypeCompound = s_defaultTypeCompound;
		const char* setterTypeCompound = s_defaultTypeCompound;
		if (propertyNode->m_get)
		{
			sprintf_s(getterFunc, "%s_get_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
			getterTypeCompound = typeCompoundToString(propertyNode->m_get->m_typeCompound);
		}
		else
		{
			strcpy_s(getterFunc, "nullptr");
		}
		if (propertyNode->m_set)
		{
			sprintf_s(setterFunc, "%s_set_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
			setterTypeCompound = typeCompoundToString(propertyNode->m_set->m_typeCompound);
		}
		else
		{
			strcpy_s(setterFunc, "nullptr");
		}

		if (propertyNode->isArray())
		{
			char sizeFunc[256];
			sprintf_s(sizeFunc, "%s_size_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());

			if (isStatic)
			{
				sprintf_s(buf, "::pafcore::StaticProperty(\"%s\", %s, %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					getterTypeCompound, setterTypeCompound,
					sizeFunc, getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::pafcore::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					getterTypeCompound, setterTypeCompound,
					sizeFunc, getterFunc, setterFunc);
			}
		}
		else if (propertyNode->isCollection())
		{
			char iterateFunc[256];

			sprintf_s(iterateFunc, "%s_iterate_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());

			if (isStatic)
			{
				sprintf_s(buf, "::pafcore::StaticProperty(\"%s\", %s, %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					getterTypeCompound, setterTypeCompound,
					iterateFunc, getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::pafcore::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					getterTypeCompound, setterTypeCompound,
					iterateFunc, getterFunc, setterFunc);
			}
		}
		else
		{
			assert(propertyNode->isSimple());
			if (isStatic)
			{
				sprintf_s(buf, "::pafcore::StaticProperty(\"%s\", %s, %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType,
					getterTypeCompound, setterTypeCompound,
					getterFunc, setterFunc);
			}
			else
			{
				sprintf_s(buf, "::pafcore::InstanceProperty(\"%s\", %s, GetSingleton(), %s, %s, %s, %s, %s),\n",
					propertyNode->m_name->m_str.c_str(), strAttributes, valueType, 
					getterTypeCompound, setterTypeCompound,
					getterFunc, setterFunc);
			}
		}
		writeStringToFile(buf, file, indentation + 1);
	}

	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticProperties = s_staticProperties;\n", file, indentation);
		writeStringToFile("m_staticPropertyCount = paf_array_size_of(s_staticProperties);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceProperties = s_instanceProperties;\n", file, indentation);
		writeStringToFile("m_instancePropertyCount = paf_array_size_of(s_instanceProperties);\n", file, indentation);
	}
}

void writeMetaConstructor_Method_Result(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, size_t index, FILE* file, int indentation)
{
	char buf[4096];

	const char* typeCompound = typeCompoundToString(methodNode->m_resultTypeCompound);
	if (methodNode->m_resultTypeName)
	{
		assert(snt_type_name == methodNode->m_resultTypeName->m_nodeType);
		std::string typeName;
		TypeCategory typeCategory = CalcTypeNativeName(typeName, (TypeNameNode*)methodNode->m_resultTypeName, templateArguments);
		sprintf_s(buf, "::pafcore::Result(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s),\n",
			typeName.c_str(), typeCompound);
	}
	else
	{
		sprintf_s(buf, "::pafcore::Result(nullptr, %s),\n",
			typeCompound);
	}
	writeStringToFile(buf, file, indentation);
}

void writeMetaConstructor_Method_Arguments(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, size_t index, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	assert(methodNode->isStatic() == isStatic);
	size_t paramCount = methodNode->getParameterCount();

	if (0 < paramCount)
	{
		std::vector<ParameterNode*> parameterNodes;
		methodNode->m_parameterList->collectParameterNodes(parameterNodes);
		assert(parameterNodes.size() == paramCount);

		for (size_t i = 0; i < paramCount; ++i)
		{
			ParameterNode* parameterNode = parameterNodes[i];
			const char* passing = parameterPassingToString(parameterNode->m_passing);
			const char* typeCompound = typeCompoundToString(parameterNode->m_typeCompound);

			TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, templateArguments);
			sprintf_s(buf, "::pafcore::Argument(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), %s, %s),\n",
				parameterNode->m_name->m_str.c_str(), typeName.c_str(), typeCompound, passing);
			writeStringToFile(buf, file, indentation);
		}
	}
}

void writeMetaConstructor_Methods(ClassNode* classNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<MethodNode*>& methodNodes, bool isStatic, FILE* file, int indentation)
{
	char buf[4096];

	if (methodNodes.empty())
	{
		return;
	}

	size_t count = methodNodes.size();
	
	//Results
	if (isStatic)
	{
		writeStringToFile("static ::pafcore::Result s_staticResults[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::pafcore::Result s_instanceResults[] = \n", file, indentation);
	}
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		writeMetaConstructor_Method_Result(classNode, templateArguments, methodNode, i, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	//Arguments
	bool hasArguments = false;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t paramCount = methodNode->getParameterCount();
		if (paramCount > 0)
		{
			hasArguments = true;
			break;
		}
	}

	if (hasArguments)
	{
		if (isStatic)
		{
			writeStringToFile("static ::pafcore::Argument s_staticArguments[] = \n", file, indentation);
		}
		else
		{
			writeStringToFile("static ::pafcore::Argument s_instanceArguments[] = \n", file, indentation);
		}
		writeStringToFile("{\n", file, indentation);
		for (size_t i = 0; i < count; ++i)
		{
			MethodNode* methodNode = methodNodes[i];
			writeMetaConstructor_Method_Arguments(classNode, templateArguments, methodNode, i, isStatic, file, indentation + 1);
		}
		writeStringToFile("};\n", file, indentation);
	}

	//Method
	if (isStatic)
	{
		writeStringToFile("static ::pafcore::StaticMethod s_staticMethods[] = \n", file, indentation);
	}
	else
	{
		writeStringToFile("static ::pafcore::InstanceMethod s_instanceMethods[] = \n", file, indentation);
	}
	writeStringToFile("{\n", file, indentation);

	size_t argumentOffset = 0;
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		size_t parameterCount = methodNode->getParameterCount();
		char strAttributes[256];
		const char* methodName = methodNode->m_name->m_str.c_str();
		auto it = attributesOffsets.find(methodNode);
		if (it != attributesOffsets.end())
		{
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}
		char strArguments[256];
		if (parameterCount > 0)
		{
			sprintf_s(strArguments, "&%s[%zd]", isStatic ? "s_staticArguments" : "s_instanceArguments", argumentOffset);
		}
		else
		{
			strcpy_s(strArguments, "nullptr");
		}

		sprintf_s(buf, "::pafcore::%s(\"%s\", %s, %s_%s, &%s[%zd], %s, %zd, %d),\n",
			isStatic ? "StaticMethod" : "InstanceMethod",
			methodName, strAttributes, classNode->m_name->m_str.c_str(), methodName,
			isStatic ? "s_staticResults" : "s_instanceResults", i,
			strArguments, parameterCount, 0);
		writeStringToFile(buf, file, indentation + 1);
		argumentOffset += parameterCount;
	}
	writeStringToFile("};\n", file, indentation);

	if (isStatic)
	{
		writeStringToFile("m_staticMethods = s_staticMethods;\n", file, indentation);
		writeStringToFile("m_staticMethodCount = paf_array_size_of(s_staticMethods);\n", file, indentation);
	}
	else
	{
		writeStringToFile("m_instanceMethods = s_instanceMethods;\n", file, indentation);
		writeStringToFile("m_instanceMethodCount = paf_array_size_of(s_instanceMethods);\n", file, indentation);
	}
}

bool MethodNodeNameEqual(MethodNode* arg1, MethodNode* arg2)
{
	return arg1->m_name->m_str == arg2->m_name->m_str;
}

void writeMetaConstructor_Member(
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	const std::vector<MethodNode*>& staticMethodNodes_,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	const std::vector<MethodNode*>& methodNodes_,
	FILE* file, int indentation)
{
	char buf[4096];

	std::vector<MethodNode*> staticMethodNodes = staticMethodNodes_;
	std::vector<MethodNode*> methodNodes = methodNodes_;

	auto it = std::unique(staticMethodNodes.begin(), staticMethodNodes.end(), MethodNodeNameEqual);
	staticMethodNodes.erase(it, staticMethodNodes.end());

	it = std::unique(methodNodes.begin(), methodNodes.end(), MethodNodeNameEqual);
	methodNodes.erase(it, methodNodes.end());

	if (nestedTypeNodes.empty() && nestedTypeAliasNodes.empty() &&
		staticFieldNodes.empty() && staticPropertyNodes.empty() && staticMethodNodes.empty()
		&& fieldNodes.empty() && propertyNodes.empty() && methodNodes.empty())
	{
		return;
	}


	writeStringToFile("static Metadata* s_members[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	size_t nestedTypeCount = nestedTypeNodes.size();
	size_t nestedTypeAliasCount = nestedTypeAliasNodes.size();
	size_t staticFieldCount = staticFieldNodes.size();
	size_t staticPropertyCount = staticPropertyNodes.size();
	size_t staticMethodCount = staticMethodNodes.size();
	size_t fieldCount = fieldNodes.size();
	size_t propertyCount = propertyNodes.size();
	size_t methodCount = methodNodes.size();

	size_t currentNestedType = 0;
	size_t currentNestedTypeAlias = 0;
	size_t currentStaticField = 0;
	size_t currentStaticProperty = 0;
	size_t currentStaticMethod = 0;
	size_t currentField = 0;
	size_t currentProperty = 0;
	size_t currentMethod = 0;

	enum MemberCategory
	{
		unknown_member,
		nested_type,
		nested_type_alias,
		static_field,
		static_property,
		static_method,
		instance_field,
		instance_property,
		instance_method,
	};
	while (true)
	{
		MemberNode* current = 0;
		MemberCategory category = unknown_member;
		if (currentNestedType < nestedTypeCount)
		{
			current = nestedTypeNodes[currentNestedType];
			category = nested_type;
		}
		if (currentNestedTypeAlias < nestedTypeAliasCount)
		{
			MemberNode* memberNode = nestedTypeAliasNodes[currentNestedTypeAlias];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = nested_type_alias;
			}
		}
		if (currentStaticField < staticFieldCount)
		{
			MemberNode* memberNode = staticFieldNodes[currentStaticField];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_field;
			}
		}
		if (currentStaticProperty < staticPropertyCount)
		{
			MemberNode* memberNode = staticPropertyNodes[currentStaticProperty];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_property;
			}
		}
		if (currentStaticMethod < staticMethodCount)
		{
			MemberNode* memberNode = staticMethodNodes[currentStaticMethod];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = static_method;
			}
		}
		if (currentField < fieldCount)
		{
			MemberNode* memberNode = fieldNodes[currentField];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_field;
			}
		}
		if (currentProperty < propertyCount)
		{
			MemberNode* memberNode = propertyNodes[currentProperty];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_property;
			}
		}
		if (currentMethod < methodCount)
		{
			MemberNode* memberNode = methodNodes[currentMethod];
			if (0 == current || memberNode->m_name->m_str < current->m_name->m_str)
			{
				current = memberNode;
				category = instance_method;
			}
		}
		if (unknown_member == category)
		{
			break;
		}
		switch (category)
		{
		case nested_type:
			sprintf_s(buf, "s_nestedTypes[%zd],\n", currentNestedType);
			++currentNestedType;
			break;
		case nested_type_alias:
			sprintf_s(buf, "s_nestedTypeAliases[%zd],\n", currentNestedTypeAlias);
			++currentNestedTypeAlias;
			break;
		case static_field:
			sprintf_s(buf, "&s_staticFields[%zd],\n", staticFieldNodes[currentStaticField]->m_orderIndex);
			++currentStaticField;
			break;
		case static_property:
			sprintf_s(buf, "&s_staticProperties[%zd],\n", staticPropertyNodes[currentStaticProperty]->m_orderIndex);
			++currentStaticProperty;
			break;
		case static_method:
			sprintf_s(buf, "&s_staticMethods[%zd],\n", currentStaticMethod);
			++currentStaticMethod;
			break;
		case instance_field:
			sprintf_s(buf, "&s_instanceFields[%zd],\n", fieldNodes[currentField]->m_orderIndex);
			++currentField;
			break;
		case instance_property:
			sprintf_s(buf, "&s_instanceProperties[%zd],\n", propertyNodes[currentProperty]->m_orderIndex);
			++currentProperty;
			break;
		case instance_method:
			sprintf_s(buf, "&s_instanceMethods[%zd],\n", currentMethod);
			++currentMethod;
			break;
		default:
			assert(false);
		}
		writeStringToFile(buf, file, indentation + 1);
	}

	writeStringToFile("};\n", file, indentation);
	writeStringToFile("m_members = s_members;\n", file, indentation);
	writeStringToFile("m_memberCount = paf_array_size_of(s_members);\n", file, indentation);

}

void writeMetaConstructor_BaseClasses(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::vector<std::pair<TokenNode*, TypeNameNode*>> tempNodes;
	classNode->m_baseList->collectTypeNameNodesNotNoMeta(tempNodes);
	std::vector<TypeNameNode*> typeNameNodes;
	size_t count = tempNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = tempNodes[i].second;
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (!typeNode->getSyntaxNode()->isNoMeta())
		{
			typeNameNodes.push_back(typeNameNode);
		}
	}

	if (typeNameNodes.empty())
	{
		return;
	}
	std::string className;
	classNode->getNativeName(className, templateArguments);

	writeStringToFile("static BaseClass s_baseClasses[] =\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	
	count = typeNameNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];

		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
			sprintf_s(buf, "{RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), paf_base_offset_of(%s, %s)},\n",
				typeName.c_str(), className.c_str(), typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);	
	}
	writeStringToFile("};\n", file, indentation);
	writeStringToFile("m_baseClasses = s_baseClasses;\n", file, indentation);
	writeStringToFile("m_baseClassCount = paf_array_size_of(s_baseClasses);\n", file, indentation);
}

void writeMetaConstructor_ClassTypeIterators(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::vector<std::pair<TokenNode*, TypeNameNode*>> tempNodes;
	classNode->m_baseList->collectTypeNameNodesNotNoMeta(tempNodes);
	std::vector<TypeNameNode*> typeNameNodes;
	size_t count = tempNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = tempNodes[i].second;
		TypeNode* typeNode = typeNameNode->getActualTypeNode(templateArguments);
		if (!typeNode->getSyntaxNode()->isNoMeta())
		{
			typeNameNodes.push_back(typeNameNode);
		}
	}

	if (typeNameNodes.empty())
	{
		return;
	}
	//std::string className;
	//classNode->getNativeName(className, templateArguments);

	writeStringToFile("static ::pafcore::ClassTypeIterator s_classTypeIterators[] =\n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	count = typeNameNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];
		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
		sprintf_s(buf, "::pafcore::ClassTypeIterator(RuntimeTypeOf<%s>::RuntimeType::GetSingleton()->m_firstDerivedClass, this),\n",
			typeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);
	for (size_t i = 0; i < count; ++i)
	{
		TypeNameNode* typeNameNode = typeNameNodes[i];
		TypeCategory typeCategory = CalcTypeNativeName(typeName, typeNameNode, templateArguments);
		sprintf_s(buf, "RuntimeTypeOf<%s>::RuntimeType::GetSingleton()->m_firstDerivedClass = &s_classTypeIterators[%zd];\n",
			typeName.c_str(), i);
		writeStringToFile(buf, file, indentation);
	}
	writeStringToFile("m_classTypeIterators = s_classTypeIterators;\n", file, indentation);
}

void writeMetaConstructor_NestedTypes(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MemberNode*>& nestedTypeNodes, FILE* file, int indentation)
{
	char buf[4096];
	if (nestedTypeNodes.empty())
	{
		return;
	}
	size_t count = nestedTypeNodes.size();
	writeStringToFile("static ::pafcore::Type* s_nestedTypes[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* typeNode = nestedTypeNodes[i];
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, typeNode, templateArguments);

		sprintf_s(buf, "%s::GetSingleton(),\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_nestedTypes = s_nestedTypes;\n", file, indentation);
	writeStringToFile("m_nestedTypeCount = paf_array_size_of(s_nestedTypes);\n", file, indentation);
}

void writeMetaConstructor_NestedTypeAliases(ClassNode* classNode, TemplateArguments* templateArguments, std::vector<MemberNode*>& nestedTypeAliasNodes, FILE* file, int indentation)
{
	char buf[4096];
	if (nestedTypeAliasNodes.empty())
	{
		return;
	}
	size_t count = nestedTypeAliasNodes.size();
	writeStringToFile("static ::pafcore::TypeAlias* s_nestedTypeAliases[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);

	for (size_t i = 0; i < count; ++i)
	{
		MemberNode* typeAliasNode = nestedTypeAliasNodes[i];
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, typeAliasNode, templateArguments);
		sprintf_s(buf, "%s::GetSingleton(),\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_nestedTypeAliases = s_nestedTypeAliases;\n", file, indentation);
	writeStringToFile("m_nestedTypeAliasCount = paf_array_size_of(s_nestedTypeAliases);\n", file, indentation);
}

void writeMetaConstructor_Scope(MemberNode* memberNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	assert(0 != memberNode->m_enclosing);
	if (snt_namespace == memberNode->m_enclosing->m_nodeType)
	{
		writeStringToFile("::pafcore::NameSpace::GetGlobalNameSpace()", file, indentation);

		std::vector<ScopeNode*> enclosings;
		memberNode->getEnclosings(enclosings);
		assert(!enclosings.empty());
		size_t count = enclosings.size();
		for (size_t i = 1; i < count; ++i)//enclosings[0] is global namespace 
		{
			ScopeNode* enclosing = enclosings[i];
			if (snt_namespace == enclosing->m_nodeType)
			{
				sprintf_s(buf, "->getNameSpace(\"%s\")", enclosing->m_name->m_str.c_str());
				writeStringToFile(buf, file);
			}
			else
			{
				break;
			}
		}
		writeStringToFile("->registerMember(this);\n", file);
	}
	else if (snt_class == memberNode->m_enclosing->m_nodeType)
	{
		std::string metaTypeName;
		GetMetaTypeFullName(metaTypeName, memberNode->m_enclosing, templateArguments);
		sprintf_s(buf, "m_enclosing = %s::GetSingleton();\n", metaTypeName.c_str());
		writeStringToFile(buf, file, indentation);
	}
	else
	{
		assert(false);
	}
}

void writeMetaConstructor(ClassNode* classNode,
	TemplateArguments* templateArguments,
	std::vector<MemberNode*>& nestedTypeNodes,
	std::vector<MemberNode*>& nestedTypeAliasNodes,
	std::vector<FieldNode*>& staticFieldNodes,
	std::vector<PropertyNode*>& staticPropertyNodes,
	std::vector<MethodNode*>& staticMethodNodes,
	std::vector<FieldNode*>& fieldNodes,
	std::vector<PropertyNode*>& propertyNodes,
	std::vector<MethodNode*>& methodNodes,
	FILE* file, int indentation)
{
	char buf[4096];
	std::string localClassName;
	std::string className;
	std::string metaClassName;
	classNode->getLocalName(localClassName, templateArguments);
	classNode->getNativeName(className, templateArguments);
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);


	std::sort(nestedTypeNodes.begin(), nestedTypeNodes.end(), CompareMemberNodeByName());
	std::sort(nestedTypeAliasNodes.begin(), nestedTypeAliasNodes.end(), CompareMemberNodeByName());
	//std::sort(fieldNodes.begin(), fieldNodes.end(), CompareMemberNodeByName());
	//std::sort(propertyNodes.begin(), propertyNodes.end(), CompareMemberNodeByName());
	std::sort(methodNodes.begin(), methodNodes.end(), CompareMethodNode());
	//std::sort(staticFieldNodes.begin(), staticFieldNodes.end(), CompareMemberNodeByName());
	//std::sort(staticPropertyNodes.begin(), staticPropertyNodes.end(), CompareMemberNodeByName());
	std::sort(staticMethodNodes.begin(), staticMethodNodes.end(), CompareMethodNode());

	sprintf_s(buf, "%s::%s() : ::pafcore::ClassType(\"%s\", ::pafcore::MetaCategory::%s, \"%s\")\n",
		metaClassName.c_str(), metaClassName.c_str(),
		localClassName.c_str(),
		classNode->m_category ? classNode->m_category->m_str.c_str() : "object",
		classNode->getSourceFilePath().c_str());

	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeMetaConstructor_Attributeses(attributesOffsets, classNode, staticFieldNodes, staticPropertyNodes, staticMethodNodes,
		fieldNodes, propertyNodes, methodNodes, file, indentation + 1);

	writeMetaConstructor_attributesForType(attributesOffsets, classNode, file, indentation + 1);

	sprintf_s(buf, "m_size = sizeof(%s);\n", className.c_str());
	writeStringToFile(buf, file, indentation + 1);

	writeMetaConstructor_BaseClasses(classNode, templateArguments, file, indentation + 1);
	writeMetaConstructor_ClassTypeIterators(classNode, templateArguments, file, indentation + 1);
	writeMetaConstructor_NestedTypes(classNode, templateArguments, nestedTypeNodes, file, indentation + 1);
	writeMetaConstructor_NestedTypeAliases(classNode, templateArguments, nestedTypeAliasNodes, file, indentation + 1);
	writeMetaConstructor_Fields(classNode, templateArguments, attributesOffsets, staticFieldNodes, true, file, indentation + 1);
	writeMetaConstructor_Properties(classNode, templateArguments, attributesOffsets, staticPropertyNodes, true, file, indentation + 1);
	writeMetaConstructor_Methods(classNode, templateArguments, attributesOffsets, staticMethodNodes, true, file, indentation + 1);

	writeMetaConstructor_Fields(classNode, templateArguments, attributesOffsets, fieldNodes, false, file, indentation + 1);
	writeMetaConstructor_Properties(classNode, templateArguments, attributesOffsets, propertyNodes, false, file, indentation + 1);
	writeMetaConstructor_Methods(classNode, templateArguments, attributesOffsets, methodNodes, false, file, indentation + 1);

	std::sort(fieldNodes.begin(), fieldNodes.end(), CompareMemberNodeByName());
	std::sort(propertyNodes.begin(), propertyNodes.end(), CompareMemberNodeByName());
	std::sort(staticFieldNodes.begin(), staticFieldNodes.end(), CompareMemberNodeByName());
	std::sort(staticPropertyNodes.begin(), staticPropertyNodes.end(), CompareMemberNodeByName());

	writeMetaConstructor_Member(nestedTypeNodes, nestedTypeAliasNodes, staticFieldNodes, staticPropertyNodes,
		staticMethodNodes, fieldNodes, propertyNodes, methodNodes, file, indentation + 1);

	writeMetaConstructor_Scope(classNode, templateArguments, file, indentation + 1);

	writeStringToFile("}\n\n", file, indentation);
}

void writeEnumMetaConstructor_Enumerators(EnumNode* enumNode, TemplateArguments* templateArguments, std::map<SyntaxNodeImpl*, size_t> attributesOffsets,
	std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation)
{
	char buf[4096];
	char strAttributes[256];
	if (enumerators.empty())
	{
		return;
	}
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, enumNode, templateArguments);
	size_t count = enumerators.size();

	writeStringToFile("static ::pafcore::Enumerator s_enumerators[] = \n", file, indentation);
	writeStringToFile("{\n", file, indentation);
	for (size_t i = 0; i < count; ++i)
	{
		EnumeratorNode* enumerator = enumerators[i];
		if (enumerator->m_attributeList)
		{
			auto it = attributesOffsets.find(enumerator);
			assert(it != attributesOffsets.end());
			sprintf_s(strAttributes, "&s_attributeses[%zd]", it->second);
		}
		else
		{
			strcpy_s(strAttributes, "nullptr");
		}

		std::string enumScopeName;
		enumNode->m_enclosing->getNativeName(enumScopeName, 0);
		if (enumNode->isStronglyTypedEnum())
		{
			sprintf_s(buf, "::pafcore::Enumerator(\"%s\", %s, %s::GetSingleton(), int(%s::%s::%s)),\n",
				enumerator->m_name->m_str.c_str(), strAttributes,
				metaClassName.c_str(), enumScopeName.c_str(), enumNode->m_name->m_str.c_str(), enumerator->m_name->m_str.c_str());
		}
		else
		{
			sprintf_s(buf, "::pafcore::Enumerator(\"%s\", %s, %s::GetSingleton(), %s::%s),\n",
				enumerator->m_name->m_str.c_str(), strAttributes,
				metaClassName.c_str(), enumScopeName.c_str(), enumerator->m_name->m_str.c_str());
		}
		writeStringToFile(buf, file, indentation + 1);
	}
	writeStringToFile("};\n", file, indentation);

	writeStringToFile("m_enumerators = s_enumerators;\n", file, indentation);
	writeStringToFile("m_enumeratorCount = paf_array_size_of(s_enumerators);\n", file, indentation);
}

void writeEnumMetaConstructor(EnumNode* enumNode, TemplateArguments* templateArguments, std::vector<EnumeratorNode*>& enumerators, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	enumNode->getNativeName(typeName, 0);
	GetMetaTypeFullName(metaTypeName, enumNode, templateArguments);

	sprintf_s(buf, "%s::%s() : ::pafcore::EnumType(\"%s\", \"%s\")\n",
		metaTypeName.c_str(),
		metaTypeName.c_str(),
		enumNode->m_name->m_str.c_str(),
		enumNode->getSourceFilePath().c_str());

	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeEnumMetaConstructor_Attributeses(attributesOffsets, enumNode, enumerators, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, enumNode, file, indentation + 1);

	sprintf_s(buf, "m_size = sizeof(%s);\n", typeName.c_str());
	writeStringToFile(buf, file, indentation + 1);

	writeEnumMetaConstructor_Enumerators(enumNode, templateArguments, attributesOffsets, enumerators, file, indentation + 1);
	writeMetaConstructor_Scope(enumNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void writeMetaGetSingletonImpls(MemberNode* memberNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	char buf[4096];
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, memberNode, templateArguments);

	sprintf_s(buf, "%s* %s::GetSingleton()\n", metaClassName.c_str(), metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	sprintf_s(buf, "static %s* s_instance = 0;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	sprintf_s(buf, "static char s_buffer[sizeof(%s)];\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("if(0 == s_instance)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "s_instance = (%s*)s_buffer;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 2);
	sprintf_s(buf, "new (s_buffer)%s;\n", metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);
	writeStringToFile("return s_instance;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
}

void MetaSourceFileGenerator::generateCode_Typedef(FILE* file, TypedefNode* typedefNode, TemplateArguments* templateArguments, int indentation)
{
	if (typedefNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	typedefNode->getNativeName(typeName, templateArguments);
	GetMetaTypeFullName(metaTypeName, typedefNode, templateArguments);

	sprintf_s(buf, "%s::%s() : TypeAlias(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), \"%s\")\n",
		metaTypeName.c_str(), metaTypeName.c_str(), typedefNode->m_name->m_str.c_str(), typeName.c_str(), typedefNode->getSourceFilePath().c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeTypeDefMetaConstructor_Attributeses(attributesOffsets, typedefNode, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, typedefNode, file, indentation + 1);

	writeMetaConstructor_Scope(typedefNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
	writeMetaGetSingletonImpls(typedefNode, templateArguments, file, indentation);
}

void MetaSourceFileGenerator::generateCode_TypeDeclaration(FILE* file, TypeDeclarationNode* typeDeclarationNode, TemplateArguments* templateArguments, int indentation)
{
	if (typeDeclarationNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string typeName;
	std::string metaTypeName;
	typeDeclarationNode->getNativeName(typeName, templateArguments);
	GetMetaTypeFullName(metaTypeName, typeDeclarationNode, templateArguments);

	sprintf_s(buf, "%s::%s() : TypeAlias(\"%s\", RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), \"%s\")\n",
		metaTypeName.c_str(), metaTypeName.c_str(), typeDeclarationNode->m_name->m_str.c_str(), typeName.c_str(), typeDeclarationNode->getSourceFilePath().c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	std::map<SyntaxNodeImpl*, size_t> attributesOffsets;
	writeTypeDefMetaConstructor_Attributeses(attributesOffsets, typeDeclarationNode, file, indentation + 1);
	writeMetaConstructor_attributesForType(attributesOffsets, typeDeclarationNode, file, indentation + 1);
	writeMetaConstructor_Scope(typeDeclarationNode, templateArguments, file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);
	writeMetaGetSingletonImpls(typeDeclarationNode, templateArguments, file, indentation);
}

void writeOverrideMethodParameter(MethodNode* methodNode, ParameterNode* parameterNode, FILE* file);
std::string CalcCompoundTypeName(TypeNameNode* typeNameNode, TypeCompound typeCompound, ParameterPassing passing, ScopeNode* scopeNode);


void writeInterfaceMethodImpl_AssignThis(ClassNode* classNode, MethodNode* methodNode, FILE* file, int indentation)
{
	std::string className;
	classNode->getFullName(className, 0);
	char buf[4096];
	sprintf_s(buf, "__self__.assignRawPtr<%s>(this);\n", className.c_str());
	writeStringToFile(buf, file, indentation);
}

void writeInterfaceMethodImpl_AssignParam(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, 0);

	switch (parameterNode->m_typeCompound)
	{
	case tc_raw_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignRawPtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_raw_array:
		sprintf_s(buf, "__arguments__[%zd].assignRawArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_borrowed_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignBorrowedPtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_borrowed_array:
		sprintf_s(buf, "__arguments__[%zd].assignBorrowedArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_unique_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignUniquePtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_unique_array:
		sprintf_s(buf, "__arguments__[%zd].assignUniqueArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_shared_ptr:
		sprintf_s(buf, "__arguments__[%zd].assignSharedPtr<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	case tc_shared_array:
		sprintf_s(buf, "__arguments__[%zd].assignSharedArray<%s>(%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
		break;
	default:
		sprintf_s(buf, "__arguments__[%zd].assignRawPtr<%s>(&%s);\n", argIndex, typeName.c_str(), parameterNode->m_name->m_str.c_str());
	}
	writeStringToFile(buf, file, indentation);
}

//void writeInterfaceMethodImpl_CastOutputParam(ParameterNode* parameterNode, size_t argIndex, FILE* file, int indentation)
//{
//	char buf[4096];
//	std::string typeName;
//	TypeCategory typeCategory = CalcTypeNativeName(typeName, parameterNode->m_typeName, 0);
//	assert(parameterNode->isOutput() && parameterNode->isByPtr());
//
//	const char* sign = parameterNode->isOutputPtr() ? "" : "&";
//	switch (typeCategory)
//	{
//	case void_type:
//		sprintf_s(buf, "__arguments__[%zd].castToVoidPtr((void**)%s%s);\n",
//			argIndex, sign, parameterNode->m_name->m_str.c_str());
//		break;
//	case primitive_type:
//		sprintf_s(buf, "__arguments__[%zd].castToPrimitivePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)%s%s);\n",
//			argIndex, typeName.c_str(), sign, parameterNode->m_name->m_str.c_str());
//		break;
//	case enum_type:
//		sprintf_s(buf, "__arguments__[%zd].castToEnumPtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)%s%s);\n",
//			argIndex, typeName.c_str(), sign, parameterNode->m_name->m_str.c_str());
//		break;
//	case object_type:
//		sprintf_s(buf, "__arguments__[%zd].castToValuePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)%s%s);\n",
//			argIndex, typeName.c_str(), sign, parameterNode->m_name->m_str.c_str());
//		break;
//	case introspectable_type:
//		sprintf_s(buf, "__arguments__[%zd].castToReferencePtr(RuntimeTypeOf<%s>::RuntimeType::GetSingleton(), (void**)%s%s);\n",
//			argIndex, typeName.c_str(), sign, parameterNode->m_name->m_str.c_str());
//		break;
//	default:
//		assert(false);
//	}
//	writeStringToFile(buf, file, indentation);
//	if (parameterNode->isByIncRefPtr())
//	{
//		sprintf_s(buf, "__arguments__[%zd].unhold();\n", argIndex);
//		writeStringToFile(buf, file, indentation);
//	}
//}

void writeInterfaceMethodImpl_CastResult(MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string typeName;
	TypeCategory typeCategory = CalcTypeNativeName(typeName, methodNode->m_resultTypeName, 0);

	std::string strResultType = CalcCompoundTypeName(methodNode->m_resultTypeName, methodNode->m_resultTypeCompound, pp_value, methodNode->getProgramNode());
	if (tc_none == methodNode->m_resultTypeCompound)
	{
		if (primitive_type == typeCategory)
		{
			sprintf_s(buf, "%s __res__{};\n", strResultType.c_str());
			writeStringToFile(buf, file, indentation);
			sprintf_s(buf, "__result__.castToPrimitive<%s>(__res__);\n", typeName.c_str());
			writeStringToFile(buf, file, indentation);
			writeStringToFile("return __res__;\n", file, indentation);
		}
		else if (enum_type == typeCategory)
		{
			sprintf_s(buf, "%s __res__{};\n", strResultType.c_str());
			writeStringToFile(buf, file, indentation);
			sprintf_s(buf, "__result__.castToEnum<%s>(__res__);\n", typeName.c_str());
			writeStringToFile(buf, file, indentation);
			writeStringToFile("return __res__;\n", file, indentation);
		}
		else
		{
			sprintf_s(buf, "%s* __res__{};\n", strResultType.c_str());
			writeStringToFile(buf, file, indentation);
			sprintf_s(buf, "if(__result__.castToRawPointer<%s>(__res__))\n", typeName.c_str());
			writeStringToFile(buf, file, indentation);
			writeStringToFile("{\n", file, indentation);
			writeStringToFile("return __res__;\n", file, indentation + 1);
			writeStringToFile("}\n", file, indentation);
			writeStringToFile("else\n", file, indentation);
			writeStringToFile("{\n", file, indentation);
			sprintf_s(buf, "return %s();\n", typeName.c_str());
			writeStringToFile(buf, file, indentation + 1);
			writeStringToFile("}\n", file, indentation);
		}
	}
	else
	{
		sprintf_s(buf, "%s __res__;\n", strResultType.c_str());
		writeStringToFile(buf, file, indentation);
		switch (methodNode->m_resultTypeCompound)
		{
		case tc_raw_ptr:
			sprintf_s(buf, "__result__.castToRawPtr<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_raw_array:
			sprintf_s(buf, "__result__.castToRawArray<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_borrowed_ptr:
			sprintf_s(buf, "__result__.castToBorrowedPtr<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_borrowed_array:
			sprintf_s(buf, "__result__.castToBorrowedArray<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_unique_ptr:
			sprintf_s(buf, "__result__.castToUniquePtr<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_unique_array:
			sprintf_s(buf, "__result__.castToUniqueArray<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_shared_ptr:
			sprintf_s(buf, "__result__.castToSharedPtr<%s>(__res__);\n", typeName.c_str());
			break;
		case tc_shared_array:
			sprintf_s(buf, "__result__.castToSharedArray<%s>(__res__);\n", typeName.c_str());
			break;
		}
		writeStringToFile("return __res__;\n", file, indentation);
	}
}

void writeInterfaceMethodImpl(ClassNode* classNode, TemplateArguments* templateArguments, MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];
	std::string subclassProxyName;
	GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);
	IdentifyNode* methodNameNode = methodNode->m_nativeName ? methodNode->m_nativeName : methodNode->m_name;

	std::string resultName;
	if (methodNode->m_resultTypeName)
	{
		resultName = CalcCompoundTypeName(methodNode->m_resultTypeName, methodNode->m_resultTypeCompound, pp_value, methodNode->getProgramNode());
	}
	else if(methodNode->m_voidResult)
	{
		resultName = "void";
	}
	sprintf_s(buf, "%s %s::%s( ", resultName.c_str(), subclassProxyName.c_str(), methodNameNode->m_str.c_str());
	writeStringToFile(buf, file, indentation);

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t paramCount = parameterNodes.size();
	for (size_t i = 0; i < paramCount; ++i)
	{
		if (0 != i)
		{
			writeStringToFile(", ", file);
		}
		writeOverrideMethodParameter(methodNode, parameterNodes[i], file);
	}
	writeStringToFile(")", file);
	if (methodNode->m_constant)
	{
		writeStringToFile(" const", file, indentation);
	}
	writeStringToFile("\n", file);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("::pafcore::Variant __self__, __result__;\n", file, indentation + 1);

	if (0 < paramCount)
	{
		sprintf_s(buf, "::pafcore::Variant __arguments__[%zd];\n", paramCount);
		writeStringToFile(buf, file, indentation + 1);
	}


	writeInterfaceMethodImpl_AssignThis(classNode, methodNode, file, indentation + 1);
	for (size_t i = 0; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		writeInterfaceMethodImpl_AssignParam(parameterNode, i, file, indentation + 1);
	}
	writeStringToFile("if(m_subclassInvoker)\n", file, indentation + 1);
	writeStringToFile("{\n", file, indentation + 1);
	sprintf_s(buf, "::pafcore::ErrorCode __error__ = m_subclassInvoker->invoke(\"%s\", &__result__, &__self__, %s, %zd);\n",
		methodNode->m_name->m_str.c_str(), paramCount > 0 ? "__arguments__" : "0", paramCount);
	writeStringToFile(buf, file, indentation + 2);
	writeStringToFile("}\n", file, indentation + 1);

	for (size_t i = 0; i < paramCount; ++i)
	{
		ParameterNode* parameterNode = parameterNodes[i];
		if (parameterNode->m_typeCompound != tc_none && pp_reference == parameterNode->m_passing)
		{
			//writeInterfaceMethodImpl_CastOutputParam(parameterNode, i, file, indentation + 1);
		}
	}
	if (methodNode->m_resultTypeName)
	{
		writeInterfaceMethodImpl_CastResult(methodNode, file, indentation + 1);
	}
	writeStringToFile("}\n\n", file, indentation);
}

void writeInterfaceMethodsImpl(ClassNode* classNode, TemplateArguments* templateArguments, FILE* file, int indentation)
{
	std::vector<MethodNode*> methodNodes;
	classNode->collectOverrideMethods(methodNodes, templateArguments);
	size_t count = methodNodes.size();
	for (size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		assert(snt_method == methodNode->m_nodeType && methodNode->m_override && methodNode->isVirtual());
		writeInterfaceMethodImpl(classNode, templateArguments, methodNode, file, indentation);
	}
}


void MetaSourceFileGenerator::generateCode_SubclassProxy(FILE* file, ClassNode* classNode, TemplateArguments* templateArguments, int indentation)
{
	char buf[4096];
	std::string subclassProxyName;
	GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);

	sprintf_s(buf, "%s::%s(::pafcore::SubclassInvoker* subclassInvoker)\n", subclassProxyName.c_str(), subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("m_subclassInvoker = subclassInvoker;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	sprintf_s(buf, "%s::~%s()\n", subclassProxyName.c_str(), subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("delete m_subclassInvoker;\n", file, indentation + 1);
	writeStringToFile("}\n\n", file, indentation);

	writeInterfaceMethodsImpl(classNode, templateArguments, file, indentation);
}

