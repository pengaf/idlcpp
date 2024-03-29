#include "MetaHeaderFileGenerator.h"
#include "Utility.h"
#include "SourceFile.h"
#include "HeaderFileGenerator.h"
#include "ProgramNode.h"
#include "NamespaceNode.h"
#include "TokenNode.h"
#include "IdentifyNode.h"
#include "EnumeratorListNode.h"
#include "MemberListNode.h"
#include "EnumNode.h"
#include "ClassNode.h"
#include "TemplateClassInstanceNode.h"
#include "TemplateParametersNode.h"
#include "TypedefNode.h"
#include "typeDeclarationNode.h"
#include "TypeNameListNode.h"
#include "TypeNameNode.h"
#include "FieldNode.h"
#include "PropertyNode.h"
#include "MethodNode.h"
#include "OperatorNode.h"
#include "ParameterNode.h"
#include "ParameterListNode.h"
#include "VariableTypeNode.h"
#include "VariableTypeListNode.h"
#include "TypeTree.h"
#include "Platform.h"
#include "Options.h"
#include "CommonFuncs.h"
#include "Compiler.h"
#include <assert.h>
#include <algorithm>


std::string CalcCompoundTypeName(TypeNameNode* typeNameNode, TypeCompound typeCompound, ParameterPassing passing, ScopeNode* scopeNode)
{
	//if (nullptr == typeNameNode)
	//{
	//	return std::string("void");
	//}
	std::string name;
	switch (typeCompound)
	{
	case tc_shared_ptr:
		name = "::paf::SharedPtr<";
		break;
	case tc_shared_array:
		name = "::paf::SharedArray<";
		break;
	}
	std::string typeName;
	typeNameNode->getRelativeName(typeName, scopeNode);
	name += typeName;

	switch (typeCompound)
	{
	case tc_raw_ptr:
		name += "*";
		break;
	case tc_shared_ptr:
		name += ">";
		break;
	case tc_shared_array:
		name += ">";
		break;
	}

	switch (passing)
	{
	case pp_value:
		name += " ";
		break;
	case pp_reference:
		name += "& ";
		break;
	case pp_const_reference:
		name += " const & ";
		break;
	}
	return name;
}

const char g_metaMethodPrefix[] = "static ::paf::ErrorCode ";
const char g_metaMethodPostfix[] = "(::paf::Variant* results, uint32_t& numResults, ::paf::Variant** args, uint32_t numArgs);\n";

void writeMetaMethodDecl(const char* funcName, FILE* file, int indentation)
{
	writeStringToFile(g_metaMethodPrefix, sizeof(g_metaMethodPrefix) - 1, file, indentation);
	writeStringToFile(funcName, file);
	writeStringToFile(g_metaMethodPostfix, sizeof(g_metaMethodPostfix) - 1, file);
}

void writeMetaMethodDecls(ClassNode* classNode, std::vector<MethodNode*> methodNodes, FILE* file, int indentation)
{
	size_t methodCount = methodNodes.size();
	if(0 < methodCount)
	{
		writeStringToFile("public:\n", file, indentation);
		for(size_t i = 0 ; i < methodCount; ++i)
		{
			char funcName[256];
			if(i > 0 && methodNodes[i]->m_name->m_str == methodNodes[i-1]->m_name->m_str)
			{
				continue;
			}
			sprintf_s(funcName, "%s_%s", classNode->m_name->m_str.c_str(), methodNodes[i]->m_name->m_str.c_str());
			writeMetaMethodDecl(funcName, file, indentation + 1);
		}
	}
}

const char* g_metaPropertyDeclPrefix = "static ::paf::ErrorCode ";

const char* g_metaSimplePropertyGetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Variant* value);\n";
const char* g_metaSimplePropertySetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Variant* value);\n";

const char* g_metaArrayPropertyGetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t index, ::paf::Variant* value);\n";
const char* g_metaArrayPropertySetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t index, ::paf::Variant* value);\n";
const char* g_metaArrayPropertySizeDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, size_t& size);\n";

const char* g_metaCollectionPropertyGetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Iterator* iterator, ::paf::Variant* value);\n";
const char* g_metaCollectionPropertySetDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::Iterator* iterator, size_t removeCount, ::paf::Variant* value);\n";
const char* g_metaCollectionPropertyIterateDeclPostfix = "(::paf::InstanceProperty* instanceProperty, ::paf::Variant* that, ::paf::SharedPtr<::paf::Iterator>& iterator);\n";


const char* g_metaStaticSimplePropertyGetDeclPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Variant* value);\n";
const char* g_metaStaticSimplePropertySetDeclPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Variant* value);\n";

const char* g_metaStaticArrayPropertyGetDeclPostfix = "(::paf::StaticProperty* staticProperty, size_t index, ::paf::Variant* value);\n";
const char* g_metaStaticArrayPropertySetDeclPostfix = "(::paf::StaticProperty* staticProperty, size_t index, ::paf::Variant* value);\n";
const char* g_metaStaticArrayPropertySizeDeclPostfix = "(::paf::StaticProperty* staticProperty, size_t& size);\n";

const char* g_metaStaticCollectionPropertyGetDeclPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Iterator* iterator, ::paf::Variant* value);\n";
const char* g_metaStaticCollectionPropertySetDeclPostfix = "(::paf::StaticProperty* staticProperty, ::paf::Iterator* iterator, size_t removeCount, ::paf::Variant* value);\n";
const char* g_metaStaticCollectionPropertyDeclIteratePostfix = "(::paf::StaticProperty* staticProperty, ::paf::SharedPtr<::paf::Iterator>& iterator);\n";



void writeMetaPropertyDeclGet(const char* funcName, bool isStatic, PropertyCategory category, FILE* file, int indentation)
{
	writeStringToFile(g_metaPropertyDeclPrefix, file, indentation);
	writeStringToFile(funcName, file);
	if(isStatic)
	{
		switch (category)
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertyGetDeclPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertyGetDeclPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertyGetDeclPostfix, file);
			break;
		}
	}
	else
	{
		switch (category)
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertyGetDeclPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertyGetDeclPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertyGetDeclPostfix, file);
			break;
		}
	}
}


void writeMetaPropertyDeclSet(const char* funcName, bool isStatic, PropertyCategory category, FILE* file, int indentation)
{
	writeStringToFile(g_metaPropertyDeclPrefix, file, indentation);
	writeStringToFile(funcName, file);
	if (isStatic)
	{
		switch (category)
		{
		case simple_property:
			writeStringToFile(g_metaStaticSimplePropertySetDeclPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaStaticArrayPropertySetDeclPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaStaticCollectionPropertySetDeclPostfix, file);
			break;
		}
	}
	else
	{
		switch (category)
		{
		case simple_property:
			writeStringToFile(g_metaSimplePropertySetDeclPostfix, file);
			break;
		case array_property:
			writeStringToFile(g_metaArrayPropertySetDeclPostfix, file);
			break;
		case collection_property:
			writeStringToFile(g_metaCollectionPropertySetDeclPostfix, file);
			break;
		}
	}
}

void writeMetaPropertyDecl(ClassNode* classNode, PropertyNode* propertyNode, FILE* file, int indentation)
{
	char funcName[256];
	if(0 != propertyNode->m_get)
	{
		sprintf_s(funcName, "%s_get_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
		writeMetaPropertyDeclGet(funcName, propertyNode->isStatic(), propertyNode->getCategory(), file, indentation + 1);
	}		
	if(0 != propertyNode->m_set)
	{
		sprintf_s(funcName, "%s_set_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
		writeMetaPropertyDeclSet(funcName, propertyNode->isStatic(), propertyNode->getCategory(), file, indentation + 1);
	}
	if(propertyNode->isArray() || propertyNode->isCollection())
	{
		sprintf_s(funcName, "%s_size_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
		writeStringToFile(g_metaPropertyDeclPrefix, file, indentation + 1);
		writeStringToFile(funcName, file);
		if (propertyNode->isStatic())
		{
			writeStringToFile(g_metaStaticArrayPropertySizeDeclPostfix, file);
		}
		else
		{
			writeStringToFile(g_metaArrayPropertySizeDeclPostfix, file);
		}
	}
	if (propertyNode->isCollection())
	{
		if (propertyNode->isStatic())
		{
			sprintf_s(funcName, "%s_iterate_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
			writeStringToFile(g_metaPropertyDeclPrefix, file, indentation + 1);
			writeStringToFile(funcName, file);
			writeStringToFile(g_metaStaticCollectionPropertyDeclIteratePostfix, file);
		}
		else
		{
			sprintf_s(funcName, "%s_iterate_%s", classNode->m_name->m_str.c_str(), propertyNode->m_name->m_str.c_str());
			writeStringToFile(g_metaPropertyDeclPrefix, file, indentation + 1);
			writeStringToFile(funcName, file);
			writeStringToFile(g_metaCollectionPropertyIterateDeclPostfix, file);
		}
	}
}

void writeMetaPropertyDecls(ClassNode* classNode, std::vector<PropertyNode*> propertyNodes, FILE* file, int indentation)
{
	size_t propertyCount = propertyNodes.size();
	if(0 < propertyCount)
	{
		writeStringToFile("public:\n", file, indentation);
		for(size_t i = 0 ; i < propertyCount; ++i)
		{
			writeMetaPropertyDecl(classNode, propertyNodes[i], file, indentation);
		}
	}
}

void MetaHeaderFileGenerator::generateCode(FILE* dstFile, SourceFile* sourceFile, const char* fullPathName, const char* baseName)
{
	generateCode_Program(dstFile, sourceFile, sourceFile->m_syntaxTree, fullPathName, baseName);
}

void MetaHeaderFileGenerator::generateCode_Program(FILE* file, SourceFile* sourceFile, ProgramNode* programNode, const char* fileName, const char* cppName)
{
	char buf[4096];
	std::string pafcorePath;
	GetRelativePath(pafcorePath, fileName, g_options.m_pafcorePath.c_str());
	FormatPathForInclude(pafcorePath);

	writeStringToFile("#pragma once\n\n", file);

	if(0 == programNode->m_memberList)
	{
		return;
	}

	g_compiler.outputUsedTypesForMetaHeader(file, sourceFile);
	sprintf_s(buf, "#include \"%s.h\"\n", cppName);
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sClassType.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sEnumType.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sTypeAlias.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);
	sprintf_s(buf, "#include \"%sVariant.h\"\n", pafcorePath.c_str());
	writeStringToFile(buf, file);

	writeStringToFile("\nnamespace idlcpp\n{\n\n", file);
	generateCode_Namespace(file, programNode, 1);
	writeStringToFile("}\n\n", file);


	std::vector<TypeNode*> typeNodes;
	CollectTypeNodes(typeNodes, programNode);
	std::reverse(typeNodes.begin(), typeNodes.end());
	size_t typeCount = typeNodes.size();

	for(size_t i = 0; i < typeCount; ++i)
	{
		TypeNode* typeNode = typeNodes[i];
		if(!typeNode->isTypedef() 
			&& !typeNode->isTypeDeclaration()
			&& typeNode->getSyntaxNode()->canGenerateMetaCode())
		{
			std::string typeName;
			typeNode->getNativeName(typeName);
			std::string metaTypeName;
			GetMetaTypeFullName(metaTypeName, typeNode);
			sprintf_s(buf, "template<>\n"
				"struct RuntimeTypeOf<%s>\n"
				"{\n"
				"\ttypedef ::idlcpp::%s RuntimeType;\n"
				"};\n\n",
				typeName.c_str(), metaTypeName.c_str());
			writeStringToFile(buf, file);
		}
	}
}

void MetaHeaderFileGenerator::generateCode_Namespace(FILE* file, NamespaceNode* namespaceNode, int indentation)
{
	if (namespaceNode->isNoMeta())
	{
		return;
	}
	std::vector<MemberNode*> memberNodes;
	namespaceNode->m_memberList->collectMemberNodes(memberNodes);
	size_t count = memberNodes.size();
	for(size_t i = 0; i < count; ++i)
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


void MetaHeaderFileGenerator::generateCode_Enum(FILE* file, EnumNode* enumNode, TemplateArguments* templateArguments, int indentation)
{
	if (enumNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string metaTypeName;
	GetMetaTypeFullName(metaTypeName, enumNode, templateArguments);

	sprintf_s(buf, "class %s : public ::paf::EnumType\n",
		metaTypeName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("public:\n", file, indentation);

	writeStringToFile(metaTypeName.c_str(), metaTypeName.length(), file, indentation + 1);
	writeStringToFile("();\n", file);

	writeStringToFile("public:\n", file, indentation);
	sprintf_s(buf, "%s static %s* GetSingleton();\n", 
		g_options.m_exportMacro.c_str(), metaTypeName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("};\n\n", file, indentation);
}

void MetaHeaderFileGenerator::generateCode_Class(FILE* file, ClassNode* classNode, TemplateClassInstanceNode* templateClassInstance, int indentation)
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

	char buf[4096];
	std::string metaClassName;
	GetMetaTypeFullName(metaClassName, classNode, templateArguments);

	sprintf_s(buf, "class %s : public ::paf::ClassType\n",
		metaClassName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	std::vector<MemberNode*> memberNodes;
	std::vector<MethodNode*> methodNodes;
	std::vector<MethodNode*> staticMethodNodes;
	std::vector<PropertyNode*> propertyNodes;
	std::vector<PropertyNode*> staticPropertyNodes;
	std::vector<MemberNode*> subTypeNodes;
	std::vector<AssignOperatorNode*> assignOperatorNodes;
	std::vector<CastOperatorNode*> castOperatorNodes;

	classNode->m_memberList->collectMemberNodes(memberNodes);
	size_t memberCount = memberNodes.size();
	for(size_t i = 0; i < memberCount; ++i)
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

		if(!memberNode->isNoMeta())
		{
			if(snt_method == memberNode->m_nodeType)
			{
				MethodNode* methodNode = static_cast<MethodNode*>(memberNode);
				if(memberNode->m_name->m_str != classNode->m_name->m_str)
				{
					if(methodNode->isStatic())
					{
						staticMethodNodes.push_back(methodNode);
					}
					else
					{
						methodNodes.push_back(methodNode);
					}
				}
			}
			else if(snt_property == memberNode->m_nodeType)
			{
				PropertyNode* propertyNode = static_cast<PropertyNode*>(memberNode);
				if(propertyNode->isStatic())
				{
					staticPropertyNodes.push_back(propertyNode);
				}
				else
				{
					propertyNodes.push_back(propertyNode);
				}
			}
			else if(snt_enum == memberNode->m_nodeType
				|| snt_class == memberNode->m_nodeType
				|| snt_typedef == memberNode->m_nodeType
				|| snt_type_declaration == memberNode->m_nodeType)
			{
				subTypeNodes.push_back(memberNode);
			}
			else if (snt_operator == memberNode->m_nodeType)
			{
				switch (static_cast<OperatorNode*>(memberNode)->m_operatorCategory)
				{
				case assgin_operator:
					assignOperatorNodes.push_back(static_cast<AssignOperatorNode*>(memberNode));
					break;
				case cast_operator:
					castOperatorNodes.push_back(static_cast<CastOperatorNode*>(memberNode));
					break;
				}
			}
			else
			{
				assert(snt_field == memberNode->m_nodeType);
			}
		}
	}

	std::sort(propertyNodes.begin(), propertyNodes.end(), CompareMemberNodeByName());
	std::sort(methodNodes.begin(), methodNodes.end(), CompareMemberNodeByName());
	std::sort(staticPropertyNodes.begin(), staticPropertyNodes.end(), CompareMemberNodeByName());
	std::sort(staticMethodNodes.begin(), staticMethodNodes.end(), CompareMemberNodeByName());


	writeStringToFile("public:\n", file, indentation);
	writeStringToFile(metaClassName.c_str(), metaClassName.length(), file, indentation + 1);
	writeStringToFile("();\n", file);

	if (nullptr == classNode->m_category || classNode->m_category->m_str == "object" || classNode->m_category->m_str == "string" || classNode->m_category->m_str == "buffer")
	{
		writeStringToFile("public:\n", file, indentation);
		writeStringToFile("virtual ::paf::ErrorCode construct(void* address, ::paf::Variant** args, uint32_t numArgs);\n", file, indentation + 1);
		writeStringToFile("virtual bool constructArray(void* address, size_t count) override;\n", file, indentation + 1);
		writeStringToFile("virtual bool destruct(void* self) override;\n", file, indentation + 1);
		writeStringToFile("virtual bool copyAssign(void* self, const void* src) override;\n", file, indentation + 1);
		if (!assignOperatorNodes.empty())
		{
			writeStringToFile("virtual bool assign(void* self, ::paf::Type* srcType, const void* src) override;\n", file, indentation + 1);
		}
		if (!castOperatorNodes.empty())
		{
			writeStringToFile("virtual bool cast(::paf::Type* dstType, void* dst, const void* self) override;\n", file, indentation + 1);
		}
		if(classNode->needSubclassProxy(templateArguments))
		{
			writeStringToFile("public:\n", file, indentation);
			writeStringToFile("virtual ::paf::SharedPtr<::paf::Introspectable> createSubclassProxy(::paf::SubclassInvoker* subclassInvoker) override;\n", file, indentation + 1);
		}
	}
	writeMetaPropertyDecls(classNode, propertyNodes, file, indentation);
	writeMetaMethodDecls(classNode, methodNodes, file, indentation);
	writeMetaPropertyDecls(classNode, staticPropertyNodes, file, indentation);
	writeMetaMethodDecls(classNode, staticMethodNodes, file, indentation);


	writeStringToFile("public:\n", file, indentation);
	sprintf_s(buf, "%s static %s* GetSingleton();\n", 
		g_options.m_exportMacro.c_str(), metaClassName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("};\n\n", file, indentation);

	if(classNode->needSubclassProxy(templateArguments))
	{
		generateCode_SubclassProxy(file, classNode, templateArguments, indentation);
	}

	size_t subTypeCount = subTypeNodes.size();
	for(size_t i = 0; i < subTypeCount; ++i)
	{
		MemberNode* typeNode = subTypeNodes[i];
		switch (typeNode->m_nodeType)
		{
		case snt_enum:
			generateCode_Enum(file, static_cast<EnumNode*>(typeNode), templateArguments, indentation);
			break;
		case snt_class:
			generateCode_Class(file, static_cast<ClassNode*>(typeNode), templateClassInstance, indentation);
			break;
		case snt_typedef:
			generateCode_Typedef(file, static_cast<TypedefNode*>(typeNode), templateArguments, indentation);
			break;
		case snt_type_declaration:
			generateCode_TypeDeclaration(file, static_cast<TypeDeclarationNode*>(typeNode), templateArguments, indentation);
			break;
		default:
			assert(false);
		}
	}
}

void writeOverrideMethodParameter(MethodNode* methodNode, ParameterNode* parameterNode, FILE* file)
{
	std::string paramName = CalcCompoundTypeName(parameterNode->m_typeName, parameterNode->m_typeCompound, parameterNode->m_passing, methodNode->getProgramNode());
	writeStringToFile(paramName.c_str(), file);
	writeSpaceToFile(file);
	writeStringToFile(parameterNode->m_name->m_str.c_str(), file);
};

void writeOverrideMethodOutputParameter(MethodNode* methodNode, VariableTypeNode* resultNode, FILE* file)
{
	std::string paramName = CalcCompoundTypeName(resultNode->m_typeName, resultNode->m_typeCompound, pp_reference, methodNode->getProgramNode());
	writeStringToFile(paramName.c_str(), file);
	//writeSpaceToFile(file);
	//writeStringToFile(parameterNode->m_name->m_str.c_str(), file);
};

void writeInterfaceMethodDecl(MethodNode* methodNode, FILE* file, int indentation)
{
	char buf[4096];

	VariableTypeNode* resultNode = nullptr;
	size_t startOutputParam = 0;
	std::vector<VariableTypeNode*> resultNodes;
	if (methodNode->m_resultList)
	{
		methodNode->m_resultList->collectVariableTypeNodes(resultNodes);
		if (!methodNode->m_voidResult)
		{
			resultNode = resultNodes.front();
			startOutputParam = 1;
		}
	}
	size_t resultCount = resultNodes.size();

	std::vector<ParameterNode*> parameterNodes;
	methodNode->m_parameterList->collectParameterNodes(parameterNodes);
	size_t parameterCount = parameterNodes.size();

	std::string resultName = "void";
	if (resultNode)
	{
		resultName = CalcCompoundTypeName(resultNode->m_typeName, resultNode->m_typeCompound, pp_value, methodNode->getProgramNode());
	}
	sprintf_s(buf, "%s %s(", resultName.c_str(), methodNode->m_name->m_str.c_str());
	writeStringToFile(buf, file, indentation);

	for (size_t i = startOutputParam; i < resultCount; ++i)
	{
		writeOverrideMethodOutputParameter(methodNode, resultNodes[i], file);
		sprintf_s(buf, "output%zd", i);
		writeStringToFile(buf, file);
		if (i + 1 < resultCount || 0 != parameterCount)
		{
			writeStringToFile(", ", file);
		}
	}

	for(size_t i = 0; i < parameterCount; ++i)
	{
		if(0 != i)
		{
			writeStringToFile(", ", file);
		}
		writeOverrideMethodParameter(methodNode, parameterNodes[i], file);
	}
	writeStringToFile(")", file);
	if(methodNode->m_constant)
	{
		writeStringToFile(" const", file, indentation);
	}
	writeStringToFile(" override;\n", file);
}


void writeInterfaceMethodsDecl(FILE* file, ClassNode* classNode, TemplateArguments* templateArguments, int indentation)
{
	std::vector<MethodNode*> methodNodes;
	classNode->collectOverrideMethods(methodNodes, templateArguments);
	size_t count = methodNodes.size();
	for(size_t i = 0; i < count; ++i)
	{
		MethodNode* methodNode = methodNodes[i];
		assert(snt_method == methodNode->m_nodeType && methodNode->m_override && methodNode->isVirtual());
		writeInterfaceMethodDecl(methodNode, file, indentation);
	}
}

void MetaHeaderFileGenerator::generateCode_SubclassProxy(FILE* file, ClassNode* classNode, TemplateArguments* templateArguments, int indentation)
{
	char buf[4096];
	std::string className;
	classNode->getFullName(className, templateArguments);
	std::string subclassProxyName;
	GetSubclassProxyFullName(subclassProxyName, classNode, templateArguments);
	
	sprintf_s(buf, "class %s : public %s\n", subclassProxyName.c_str(), className.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);
	writeStringToFile("public:\n", file, indentation);
	writeStringToFile("::paf::SubclassInvoker* m_subclassInvoker;\n", file, indentation + 1);
	writeStringToFile("public:\n", file, indentation);
	sprintf_s(buf, "%s(::paf::SubclassInvoker* subclassInvoker);\n", subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	sprintf_s(buf, "~%s();\n", subclassProxyName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeInterfaceMethodsDecl(file, classNode, templateArguments, indentation + 1);
	writeStringToFile("};\n\n", file, indentation);

}

void MetaHeaderFileGenerator::generateCode_TemplateClassInstance(FILE* file, TemplateClassInstanceNode* templateClassInstance, int indentation)
{
	if (templateClassInstance->isNoMeta())
	{
		return;
	}

	ClassNode* classNode = static_cast<ClassNode*>(templateClassInstance->m_classTypeNode->m_classNode);
	generateCode_Class(file, classNode, templateClassInstance, indentation);

}

void MetaHeaderFileGenerator::generateCode_Typedef(FILE* file, TypedefNode* typedefNode, TemplateArguments* templateArguments, int indentation)
{
	if (typedefNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string metaTypeName;
	GetMetaTypeFullName(metaTypeName, typedefNode, templateArguments);

	sprintf_s(buf, "class %s : public ::paf::TypeAlias\n",
		metaTypeName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("public:\n", file, indentation);

	writeStringToFile(metaTypeName.c_str(), metaTypeName.length(), file, indentation + 1);
	writeStringToFile("();\n", file);

	writeStringToFile("public:\n", file, indentation);
	sprintf_s(buf, "%s static %s* GetSingleton();\n", 
		g_options.m_exportMacro.c_str(), metaTypeName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("};\n\n", file, indentation);
}

void MetaHeaderFileGenerator::generateCode_TypeDeclaration(FILE* file, TypeDeclarationNode* typeDeclarationNode, TemplateArguments* templateArguments, int indentation)
{
	if (typeDeclarationNode->isNoMeta())
	{
		return;
	}

	char buf[4096];
	std::string metaTypeName;
	GetMetaTypeFullName(metaTypeName, typeDeclarationNode, templateArguments);

	sprintf_s(buf, "class %s : public ::paf::TypeAlias\n",
		metaTypeName.c_str());
	writeStringToFile(buf, file, indentation);
	writeStringToFile("{\n", file, indentation);

	writeStringToFile("public:\n", file, indentation);

	writeStringToFile(metaTypeName.c_str(), metaTypeName.length(), file, indentation + 1);
	writeStringToFile("();\n", file);

	writeStringToFile("public:\n", file, indentation);
	sprintf_s(buf, "%s static %s* GetSingleton();\n",
		g_options.m_exportMacro.c_str(), metaTypeName.c_str());
	writeStringToFile(buf, file, indentation + 1);
	writeStringToFile("};\n\n", file, indentation);
}

