
set(HeaderFiles
    "AttributeListNode.h"
    "AttributeNode.h"
    "BaseClassNode.h"
    "BaseClassListNode.h"
    "ClassNode.h"
    "CommonFuncs.h"
    "Compiler.h"
    "EnumeratorListNode.h"
    "EnumeratorNode.h"
    "EnumNode.h"
    "ErrorList.h"
    "FieldNode.h"
    "HeaderFileGenerator.h"
    "IdentifyListNode.h"
    "IdentifyNode.h"
    "idlcpp.h"
    "ImportDirectory.h"
    "MemberListNode.h"
    "MemberNode.h"
    "MetaHeaderFileGenerator.h"
    "MetaSourceFileGenerator.h"
    "MethodNode.h"
    "NamespaceNode.h"
    "OperatorNode.h"
    "Options.h"
    "ParameterListNode.h"
    "ParameterNode.h"
    "Platform.h"
    "ProgramNode.h"
    "PropertyNode.h"
    "VariableTypeListNode.h"
    "VariableTypeNode.h"
    "RaiseError.h"
    "ScopeNameListNode.h"
    "ScopeNameNode.h"
    "ScopeNode.h"
    "SourceFile.h"
    "SourceFileGenerator.h"
    "SyntaxNodeImpl.h"
    "targetver.h"
    "TemplateArguments.h"
    "TemplateClassInstanceNode.h"
    "TemplateParameterListNode.h"
    "TemplateParametersNode.h"
    "TokenListNode.h"
    "TokenNode.h"
    "TypeDeclarationNode.h"
    "TypedefNode.h"
    "TypeNameListNode.h"
    "TypeNameNode.h"
    "TypeTree.h"
    "Utility.h"
    "yyfuncs.h"
)
source_group("Header Files" FILES ${HeaderFiles})



set(SourceFiles
    "AttributeListNode.cpp"
    "AttributeNode.cpp"
    "BaseClassNode.cpp"
    "BaseClassListNode.cpp"
    "ClassNode.cpp"
    "CommonFuncs.cpp"
    "Compiler.cpp"
    "EnumeratorListNode.cpp"
    "EnumeratorNode.cpp"
    "EnumNode.cpp"
    "ErrorList.cpp"
    "FieldNode.cpp"
    "HeaderFileGenerator.cpp"
    "IdentifyListNode.cpp"
    "IdentifyNode.cpp"
    "idlcpp.cpp"
    "ImportDirectory.cpp"
    "MemberListNode.cpp"
    "MemberNode.cpp"
    "MetaHeaderFileGenerator.cpp"
    "MetaSourceFileGenerator.cpp"
    "MethodNode.cpp"
    "NamespaceNode.cpp"
    "OperatorNode.cpp"
    "Options.cpp"
    "ParameterListNode.cpp"
    "ParameterNode.cpp"
    "Platform.cpp"
    "ProgramNode.cpp"
    "PropertyNode.cpp"
    "VariableTypeListNode.cpp"
    "VariableTypeNode.cpp"
    "RaiseError.cpp"
    "ScopeNameListNode.cpp"
    "ScopeNameNode.cpp"
    "SourceFile.cpp"
    "SourceFileGenerator.cpp"
    "SyntaxNodeImpl.cpp"
    "TemplateArguments.cpp"
    "TemplateClassInstanceNode.cpp"
    "TemplateParameterListNode.cpp"
    "TemplateParametersNode.cpp"
    "TokenListNode.cpp"
    "TokenNode.cpp"
    "TypeDeclarationNode.cpp"
    "TypedefNode.cpp"
    "TypeNameListNode.cpp"
    "TypeNameNode.cpp"
    "TypeTree.cpp"
    "Utility.cpp"
    "yyfuncs.cpp"
)
source_group("Source Files" FILES ${SourceFiles})
    
#message(${PROJECT_SOURCE_DIR})
#message(${CMAKE_CURRENT_SOURCE_DIR})
    
set(AllFiles
    "idlcpp.l"
    "idlcpp.y"
    "idlcpp.c"
    ${HeaderFiles}
    ${SourceFiles}
)

set (aaa [=[-o%(FullPath).c %(FullPath)]=])

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/idlcpp.l.c
	    COMMAND flex "-o%(FullPath).c" "%(FullPath)"
	    MAIN_DEPENDENCY idlcpp.l)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/idlcpp.y.c ${CMAKE_CURRENT_SOURCE_DIR}/idlcpp.y.h
	    COMMAND bison "-o%(FullPath).c" -d "%(FullPath)"
	    MAIN_DEPENDENCY "idlcpp.y")

add_executable(idlcpp ${AllFiles})
