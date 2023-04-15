/* File: codegen.cc
 * ----------------
 * Implementation for the CodeGenerator class. The methods don't do anything
 * too fancy, mostly just create objects of the various Tac instruction
 * classes and append them to the list.
 */

#include "codegen.h"
#include <string.h>
#include "tac.h"
#include "mips.h"

Location *CodeGenerator::ptrThis = new Location(fpRelative, 4, "this");

CodeGenerator::CodeGenerator()
{
    // code = new List<Instruction*>();

    // global starts at 0, +4 every time
    globalLocation = OffsetToFirstGlobal;

    // local starts at -8, -4 every time
    localLocation = OffsetToFirstLocal;

    // parameters start at 4, +4 every time
    paramLocation = OffsetToFirstParam;
}

int CodeGenerator::GetNextGlobal()
{
    int g = globalLocation;
    globalLocation += VarSize; // globalLocation + 4
    return g;
}

int CodeGenerator::GetNextLocal()
{
    int l = localLocation;
    localLocation -= VarSize; // localLocation - 4
    return l;
}

int CodeGenerator::GetNextParam()
{
    int p = paramLocation;
    paramLocation += VarSize; // paramLocation + 4
    return p;
}

int CodeGenerator::GetFrameSize()
{
    return OffsetToFirstLocal - localLocation;
}

void CodeGenerator::ResetFrame()
{
    localLocation = OffsetToFirstLocal;
    paramLocation = OffsetToFirstParam;
}

char *CodeGenerator::NewLabel()
{
    static int nextLabelNum = 0;
    char temp[10];
    sprintf(temp, "_L%d", nextLabelNum++);
    return strdup(temp);
}

Location *CodeGenerator::GenTempVar()
{
    static int nextTempNum;
    char temp[10];
    Location *result = NULL;
    sprintf(temp, "_tmp%d", nextTempNum++);
    result = new Location(fpRelative, GetNextLocal(), temp);
    Assert(result != NULL);
    return result;
}

Location *CodeGenerator::GenLoadConstant(int value)
{
    Location *result = GenTempVar();
    code.push_back(new LoadConstant(result, value));
    return result;
}

Location *CodeGenerator::GenLoadConstant(const char *s)
{
    Location *result = GenTempVar();
    code.push_back(new LoadStringConstant(result, s));
    return result;
}

Location *CodeGenerator::GenLoadLabel(const char *label)
{
    Location *result = GenTempVar();
    code.push_back(new LoadLabel(result, label));
    return result;
}

void CodeGenerator::GenAssign(Location *dst, Location *src)
{
    code.push_back(new Assign(dst, src));
}

Location *CodeGenerator::GenLoad(Location *ref, int offset)
{
    Location *result = GenTempVar();
    code.push_back(new Load(result, ref, offset));
    return result;
}

void CodeGenerator::GenStore(Location *dst, Location *src, int offset)
{
    code.push_back(new Store(dst, src, offset));
}

Location *CodeGenerator::GenBinaryOp(const char *opName, Location *op1,
                                     Location *op2)
{
    Location *result = GenTempVar();
    code.push_back(new BinaryOp(BinaryOp::OpCodeForName(opName), result, op1, op2));
    return result;
}

void CodeGenerator::GenLabel(const char *label)
{
    code.push_back(new Label(label));
}

void CodeGenerator::GenIfZ(Location *test, const char *label)
{
    code.push_back(new IfZ(test, label));
}

void CodeGenerator::GenGoto(const char *label)
{
    code.push_back(new Goto(label));
}

void CodeGenerator::GenReturn(Location *val)
{
    code.push_back(new Return(val));
}

BeginFunc *CodeGenerator::GenBeginFunc()
{
    ResetFrame();
    BeginFunc *result = new BeginFunc;
    code.push_back(result);
    return result;
}

void CodeGenerator::GenEndFunc()
{
    code.push_back(new EndFunc());
}

void CodeGenerator::GenPushParam(Location *param)
{
    code.push_back(new PushParam(param));
}

void CodeGenerator::GenPopParams(int numBytesOfParams)
{
    Assert(numBytesOfParams >= 0 && numBytesOfParams % VarSize == 0);
    if (numBytesOfParams > 0)
        code.push_back(new PopParams(numBytesOfParams));
}

Location *CodeGenerator::GenLCall(const char *label, bool fnHasReturnValue)
{
    Location *result = fnHasReturnValue ? GenTempVar() : NULL;
    code.push_back(new LCall(label, result));
    return result;
}

Location *CodeGenerator::GenACall(Location *fnAddr, bool fnHasReturnValue)
{
    Location *result = fnHasReturnValue ? GenTempVar() : NULL;
    code.push_back(new ACall(fnAddr, result));
    return result;
}

static struct _builtin
{
    const char *label;
    int numArgs;
    bool hasReturn;
} builtins[] = {
    {"_Alloc", 1, true},
    {"_ReadLine", 0, true},
    {"_ReadInteger", 0, true},
    {"_StringEqual", 2, true},
    {"_PrintInt", 1, false},
    {"_PrintString", 1, false},
    {"_PrintBool", 1, false},
    {"_Halt", 0, false}};

Location *CodeGenerator::GenBuiltInCall(BuiltIn bn, Location *arg1,
                                        Location *arg2)
{
    Assert(bn >= 0 && bn < NumBuiltIns);
    struct _builtin *b = &builtins[bn];
    Location *result = NULL;

    if (b->hasReturn)
        result = GenTempVar();
    // verify appropriate number of non-NULL arguments given
    Assert((b->numArgs == 0 && !arg1 && !arg2) || (b->numArgs == 1 && arg1 && !arg2) || (b->numArgs == 2 && arg1 && arg2));
    if (arg2)
        code.push_back(new PushParam(arg2));
    if (arg1)
        code.push_back(new PushParam(arg1));
    code.push_back(new LCall(b->label, result));
    GenPopParams(VarSize * b->numArgs);
    return result;
}

void CodeGenerator::GenVTable(const char *className,
                              List<const char *> *methodLabels)
{
    code.push_back(new VTable(className, methodLabels));
}

void CodeGenerator::DoFinalCodeGen()
{
    if (IsDebugOn("tac"))
    {
        std::list<Instruction *>::iterator p;
        for (p = code.begin(); p != code.end(); ++p)
        {
            (*p)->Print();
        }
    }
    else
    {
        Mips mips;
        mips.EmitPreamble();

        std::list<Instruction *>::iterator p;
        for (p = code.begin(); p != code.end(); ++p)
        {
            (*p)->Emit(&mips);
        }
    }
}
