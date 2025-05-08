// Declares clang::SyntaxOnlyAction.
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include <Windows.h>

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

DeclarationMatcher methodMatcher = functionDecl().bind("methods");

std::stringstream ss;

class MethodPrinter : public MatchFinder::MatchCallback {
public:
  virtual void run(const MatchFinder::MatchResult &Result) {
    SourceManager &srcMgr = Result.Context->getSourceManager();
    if (const CXXMethodDecl *md =
            Result.Nodes.getNodeAs<clang::CXXMethodDecl>("methods")) {

      SourceLocation location = md->getLocation();
      if (md->isThisDeclarationADefinition() && !md->isDefaulted() && md->isDefined() && (srcMgr.getFilename(location).contains("OpenBarnyard") || srcMgr.getFilename(location).contains("OpenJPOG"))) {
        ss << '"' << md->getQualifiedNameAsString() << '"' << ',' << '"';
        for (unsigned i = 0; i < md->getNumParams(); ++i) {
            const ParmVarDecl* param = md->getParamDecl(i);
            if (i > 0) ss << ", ";
            ss << param->getType().getAsString() << " " << param->getNameAsString();
        }
        ss << "\",";
        ss << '"' << srcMgr.getFilename(location).str() << "\"," 
           << '"' << srcMgr.getLineNumber(srcMgr.getFileID(location), srcMgr.getFileOffset(location)) << "\"" 
           << std::endl;
      }
    }
  }
};

int main(int argc, const char **argv) {
  auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }

  CommonOptionsParser &OptionsParser = ExpectedParser.get();
  
  std::ifstream inFile("toshi-marker-list.txt");
  std::vector<std::string> sourceList;

  if (inFile.is_open()) {
    std::string str;
    while (std::getline(inFile, str)) {
      sourceList.emplace_back(std::move(str));
    }
  } else {
    sourceList = OptionsParser.getSourcePathList();
  }

  ClangTool Tool(OptionsParser.getCompilations(), sourceList);

  MethodPrinter printer;
  MatchFinder finder;
  finder.addMatcher(methodMatcher, &printer);

  int status = Tool.run(newFrontendActionFactory(&finder).get());

  HANDLE hFileHandle = INVALID_HANDLE_VALUE;
  while (hFileHandle == INVALID_HANDLE_VALUE) {
    // Lock file to make sure it works with multithreading
    hFileHandle = CreateFileA("toshi-marker-out.txt", FILE_APPEND_DATA, 0, 0, OPEN_ALWAYS, 0, 0);
    Sleep(0);
  }

  std::string data = ss.str();
  DWORD writtenBytes;
  WriteFile(hFileHandle, data.c_str(), data.size(), &writtenBytes, NULL);
  
  CloseHandle(hFileHandle);
  return status;
}