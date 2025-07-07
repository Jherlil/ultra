__constant uint K[64]={
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

uint rotr(uint x,uint n){return (x>>n)|(x<<(32-n));}
uint Ch(uint x,uint y,uint z){return (x&y)^(~x&z);} 
uint Maj(uint x,uint y,uint z){return (x&y)^(x&z)^(y&z);} 
uint Sig0(uint x){return rotr(x,2)^rotr(x,13)^rotr(x,22);} 
uint Sig1(uint x){return rotr(x,6)^rotr(x,11)^rotr(x,25);} 
uint sig0(uint x){return rotr(x,7)^rotr(x,18)^(x>>3);} 
uint sig1(uint x){return rotr(x,17)^rotr(x,19)^(x>>10);} 

__kernel void sha256_33_kernel(__global const uchar *in,__global uchar* out){
    int gid=get_global_id(0);
    const uchar *d=in+gid*64;
    uint w[64];
    for(int i=0;i<16;i++){
        w[i]=((uint)d[i*4]<<24)|((uint)d[i*4+1]<<16)|((uint)d[i*4+2]<<8)|((uint)d[i*4+3]);
    }
    for(int i=16;i<64;i++)
        w[i]=w[i-16]+sig0(w[i-15])+w[i-7]+sig1(w[i-2]);
    uint a=0x6a09e667,b=0xbb67ae85,c=0x3c6ef372,d0=0xa54ff53a;
    uint e=0x510e527f,f=0x9b05688c,g=0x1f83d9ab,h=0x5be0cd19;
    for(int i=0;i<64;i++){
        uint t1=h+Sig1(e)+Ch(e,f,g)+K[i]+w[i];
        uint t2=Sig0(a)+Maj(a,b,c);
        h=g;g=f;f=e;e=d0+t1;d0=c;c=b;b=a;a=t1+t2;
    }
    uint s0=0x6a09e667+a; uint s1=0xbb67ae85+b; uint s2=0x3c6ef372+c; uint s3=0xa54ff53a+d0;
    uint s4=0x510e527f+e; uint s5=0x9b05688c+f; uint s6=0x1f83d9ab+g; uint s7=0x5be0cd19+h;
    uchar *o=out+gid*32;
    o[0]=s0>>24; o[1]=s0>>16; o[2]=s0>>8; o[3]=s0;
    o[4]=s1>>24; o[5]=s1>>16; o[6]=s1>>8; o[7]=s1;
    o[8]=s2>>24; o[9]=s2>>16; o[10]=s2>>8; o[11]=s2;
    o[12]=s3>>24; o[13]=s3>>16; o[14]=s3>>8; o[15]=s3;
    o[16]=s4>>24; o[17]=s4>>16; o[18]=s4>>8; o[19]=s4;
    o[20]=s5>>24; o[21]=s5>>16; o[22]=s5>>8; o[23]=s5;
    o[24]=s6>>24; o[25]=s6>>16; o[26]=s6>>8; o[27]=s6;
    o[28]=s7>>24; o[29]=s7>>16; o[30]=s7>>8; o[31]=s7;
}
