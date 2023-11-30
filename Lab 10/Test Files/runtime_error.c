#include<stdio.h>
int main()
{
    for(int i=0;i<10000;i++) {
        for(int j=0;j<100000;j++) {
            continue;
        }
    }
    for(int i=1;i<=20;i++)
        printf("%d ",i/0);
}
