##decl var = random;
##decl string = "Hello
World";
##print string, ' ', (var * 5);
##decl num = (random 1000, 10000);
int ##print "random_variable", num; = 5;
##decl func =  {
    print args_length, ' ', args_0, '\n';
    print "Test";
    decl num = 75;
    print num;
};
##decl add = {
    return (args_0 + args_1);
};
##exec func, 5;
int main() {
    printf("The C stuff\n");
    int num = 5 % 3;
    printf("%d \n", num);
    printf("%d \n", ##print "random_variable", num;);
    return 0;
}
##exec func, 7, 23;
##print (exec add, 55, 5);
##if (1 == 2), {
    print "True";
};