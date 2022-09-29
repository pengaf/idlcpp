#include "FieldNode.h"
#include "TokenNode.h"
#include "IdentifyNode.h"
#include "ClassNode.h"
#include "TypeNameNode.h"
#include "TypeTree.h"
#include "RaiseError.h"
#include "Compiler.h"
#include <assert.h>

FieldNode::FieldNode(TypeNameNode* typeName, TypeCompound typeCompound, IdentifyNode* name)
{
	m_nodeType = snt_field;
	m_static = 0;
	m_typeName = typeName;
	m_typeCompound = typeCompound;
	m_name = name;
	m_leftBracket = 0;
	m_rightBracket = 0;
	m_semicolon = 0;
}

bool FieldNode::isStatic()
{
	return (0 != m_static);
}

bool FieldNode::isArray()
{
	return (0 != m_leftBracket);
}

void FieldNode::checkTypeNames(TypeNode* enclosingTypeNode, TemplateArguments* templateArguments)
{
	m_typeName->calcTypeNodes(enclosingTypeNode, templateArguments);
}

void FieldNode::checkSemantic(TemplateArguments* templateArguments)
{
	MemberNode::checkSemantic(templateArguments);
	TypeNode* typeNode = m_typeName->getTypeNode(templateArguments);
	if (0 == typeNode)
	{
		return;
	}
	g_compiler.useType(typeNode, templateArguments, tc_none == m_typeCompound ? tu_use_definition : tu_use_declaration, m_typeName);
}

