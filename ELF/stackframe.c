int func5(int para1, int para2, int para3, int para4, int para5)
{
/*
    int locVar1 = para1;
    int locVar2 = para2;
    int locVar3 = para3;
    int locVar4 = para4;
    int locVar5 = para5;

    return locVar1 + locVar2 + locVar3 + locVar4 + locVar5;
*/
    return para5;
}

int func4(int para1, int para2, int para3, int para4)
{
    /*
    int locVar1 = para1;
    int locVar2 = para2;
    int locVar3 = para3;
    int locVar4 = para4;

    para1 += 1;
    para2 += 1;
    para3 += 1;
    para4 += 1;
*/
    // return locVar1 + locVar2 + locVar3 + locVar4;

    // return func5(locVar1, locVar2, locVar3, locVar4, locVar4);
    return func5(para1, para2, para3, para4, para1);
}



int main(void)
{
    int var1 = 1, var2 = 2, var3 = 3, var4 = 4;

    func4(var1, var2, var3, var4);

    return 0;
}
