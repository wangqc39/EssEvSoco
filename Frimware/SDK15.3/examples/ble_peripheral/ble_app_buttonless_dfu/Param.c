#include <stdint.h>
#include <string.h>


//对参数进行检查，如果有异常则用第三个参数进行覆盖
//在参数读取时，是被系统默认参数覆盖
//在参数写入时，是被系统中当前参数覆盖
//正确返回0
/*
ParamAddr:指向检验参数的指针 
ParamLength:参数的长度
CheckLength:命令中提取的内容长度
DefaultValuePtr:指向默认值的指针
Min:最小值
Max:最大值
*/
int32_t CheckParamIndex(unsigned char *ParamAddr, int ParamLength, int CheckLength, unsigned int DefaultValue, unsigned int Min, unsigned int Max)
{
    unsigned int Param;
    unsigned int *DefaultValuePtr;

    Param = 0;
    //对char，unsigned short int，unsigned int类型的参数进行校验
    memcpy((unsigned char *)&Param, ParamAddr, ParamLength);
    if(Param > Max || Param < Min || ParamLength != CheckLength)
    {
        //长度不正确，或者值不在有效范围内的将默认值写回给参数
        DefaultValuePtr = &DefaultValue;
         memcpy(ParamAddr, DefaultValuePtr, ParamLength);
    }
    return ParamLength;
}


int32_t CheckParamBuff(unsigned char *ParamAddr, int ParamLength, uint8_t DefaultValue, uint8_t Min, uint8_t Max)
{
    uint32_t i;
    uint8_t Param;
    uint8_t *DefaultValuePtr;
    for(i = 0; i < ParamLength; i++)
    {
        memcpy((uint8_t *)&Param, ParamAddr + i, sizeof(uint8_t));
        if(Param > Max || Param < Min)
        {
            //长度不正确，或者值不在有效范围内的将默认值写回给参数
            DefaultValuePtr = &DefaultValue;
            memcpy(ParamAddr + i, DefaultValuePtr, sizeof(uint8_t));
        }
    }
    return ParamLength;
}

unsigned int CalSum(unsigned int *StartAddr, unsigned int IntLength)
{
    int i;
    unsigned int sum = 0;
    for(i = 0; i < IntLength; i++)
    {
        sum += *StartAddr;
        StartAddr++;
    }

    return sum;
}

//将内存中变量的值实时写入到配置表缓冲的对应位置中


void WriteOneParamToTable(unsigned char *SourceAddr, int offset, int Length, unsigned char *DistTable)
{
    unsigned char *TablePtr;
    TablePtr = DistTable + offset;
    memcpy(TablePtr, SourceAddr, Length);
}

int32_t InitOneParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint32_t DefaultValue, uint32_t Min, uint32_t Max)
{
    memcpy(Param, ConfigBuff + offset, length);
    CheckParamIndex(Param, length, length, DefaultValue, Min, Max);
    return 0;
}

int32_t InitBuffParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint8_t DefaultValue, uint8_t Min, uint8_t Max)
{
    memcpy(Param, ConfigBuff + offset, length);
    CheckParamBuff(Param, length, DefaultValue, Min, Max);
    return 0;
}

