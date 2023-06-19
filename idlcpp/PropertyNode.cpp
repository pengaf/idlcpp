#include "PropertyNode.h"
#include "TypeNameNode.h"
#include "TokenNode.h"
#include "IdentifyNode.h"
#include "ClassNode.h"
#include "TypeTree.h"
#include "RaiseError.h"
#include "Compiler.h"
#include <assert.h>

GetterNode::GetterNode(TokenNode* keyword, ParameterPassing passing)
{
	m_nodeType = snt_getter;
	m_keyword = keyword;
	m_nativeName = 0;
	m_passing = passing;
}

SetterNode::SetterNode(TokenNode* keyword, ParameterPassing passing)
{
	m_nodeType = snt_setter;
	m_keyword = keyword;
	m_nativeName = 0;
	m_passing = passing;
}

PropertyNode::PropertyNode(TypeNameNode* typeName, TypeCompound typeCompound, IdentifyNode* name, PropertyCategory category)
{
	m_nodeType = snt_property;
	m_modifier = 0;
	m_typeName = typeName;
	m_name = name;
	m_get = 0;
	m_set = 0;
	m_typeCompound = typeCompound;
	m_propertyCategory = category;
}

PropertyCategory PropertyNode::getCategory()
{
	return m_propertyCategory;
}

bool PropertyNode::isStatic()
{
	return (0 != m_modifier && snt_keyword_static == m_modifier->m_nodeType);
}

bool PropertyNode::isSimple()
{
	return simple_property == m_propertyCategory;
}

bool PropertyNode::isArray()
{
	return array_property == m_propertyCategory;
}

bool PropertyNode::isCollection()
{
	return collection_property == m_propertyCategory;
}

void PropertyNode::setGetter(GetterNode* getter)
{
	assert(snt_keyword_get == getter->m_keyword->m_nodeType);
	m_get = getter;
}

void PropertyNode::setSetter(SetterNode* setter)
{
	assert(snt_keyword_set == setter->m_keyword->m_nodeType);
	m_set = setter;
}

void PropertyNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	m_typeName->calcTypeNodes(enclosingTypeNode, templateArguments);
}

void PropertyNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);

	assert(snt_class == m_enclosing->m_nodeType);
	ClassNode* classNode = static_cast<ClassNode*>(m_enclosing);
	TypeNode* typeNode = m_typeName->getTypeNode(templateArguments);
	if (0 == typeNode)
	{
		return;
	}
	bool byValue = false;
	if (m_set && (tc_none == m_typeCompound && pp_value == m_set->m_passing) )
	{
		byValue = true;
	}
	if (m_get && (tc_none == m_typeCompound && pp_value == m_get->m_passing))
	{
		byValue = true;
	}
	g_compiler.useType(typeNode, templateArguments, byValue ? tu_use_definition : tu_use_declaration, m_typeName);
}
