向GPU提交一个清除某渲染目标的命令，调用Direct3D中的ID3D12GraphicsCommandList::ClearRenderTargetView方法。



Direct3D层和硬件驱动会协作将此Direct3D命令**转换为系统中GPU可以执行的本地机器指令。**



# 组件对象模型

要获取指向某COM接口的指针，**需借助特定函数或另一COM接口的方法。**

COM对象会**统计其引用次数**，在使用完某接口时，应调用它的Release方法。



辅助用户管理COM对象的生命周期，可以使用Microsoft::WRL::ComPtr类，

可以当作是COM对象的智能指针。



常用的3个ComPtr方法如下：

1.Get：返回一个指向此底层COM接口的指针。此方法常用于把原始的COM接口指针作为参数传递给函数。



2.GetAddressOf：返回指向此底层COM接口指针的地址。



3.Reset：将此ComPtr实例设置为nullptr释放与之相关的所有引用。



# 纹理格式

纹理还具有多种mipmap层级，而GPU则会据此对它们进行特殊的处理，**例如运用过滤器和进行多重采样。**



并不是任意类型的数据元素都能用于组成纹理，它只能存储**DXGI_FORMAT枚举类型**中特定格式的数据元素。



还有无类型格式的纹理，用它来预留内存，待纹理被绑定到渲染流水线之后，再具体解释它的数据类型。

```c++
DXGI_FORMAT_R16G16B16A16_TYPELESS
```

这个无类型格式保留的是由4个16位分量组成的元素，**但是没有指出数据的具体类型。**



# 交换链和页面翻转

由硬件管理的两种纹理缓冲区，前台缓冲区和后台缓冲区。



前后台缓冲的互换操作称为**呈现(presenting)。**



前台缓冲区和后台缓冲区构成了交换链，在Direct3D中用IDXGISwapChain接口来表示。



这个接口不仅存储了前台缓冲区和后台缓冲区两种纹理，而且还提高了修改缓冲区大小

(IDXGISwapChain::ResizeBuffers)和呈现缓冲区内容(IDXGISwapChain::Present)的方法。



# 深度缓冲

深度值的范围为0.0~1.0。



深度缓冲技术的原理是计算每个像素的深度值，并执行深度测试。



# 资源与描述符

在发出绘制命令之前，我们需要将与本次绘制调用相关的资源绑定到渲染流水线上。

部分资源可能在每次绘制调用时都会有所变化，所以我们也就要每次按需更新绑定。



GPU资源并非直接与渲染流水线相绑定，而是要通过一种名为描述符的对象来**对它间接引用**。



除了指定资源数据，描述符还会为GPU解释资源。



每个描述符都有一种具体类型，此类型指明了资源的具体作用。本书常用的描述符如下：

1.CBV/SRV/UAV描述符分别表示的是常量缓冲区视图、着色器资源视图和无序访问视图。



2.采样器描述符表示的是采样器资源。



3.RTV描述符表示的是渲染目标视图资源。



4.DSV描述符表示的是深度/模板视图资源。



描述符堆存有一系列描述符。



创建描述符的最佳时机为初始化期间。



# 多重采样技术的原理

超级采样SSAA。



多重采样MSAA。4X多重采样会4倍于屏幕分辨率的深度缓冲区和后台缓冲区。



# 利用Direct3D进行多重采样

```c++
typedef struct DXGI_SAMPLE_DESC
{
	UINT Count;//每个像素的采样次数
	UINT Quality;//用户期望的图像质量级别
}DXGI_SAMPLE_DESC;
```



```c++
//Quality需要根据给定的纹理格式和采样数据，使用ID3D12Device::CheckFeatureSupport方法查询对应的质量级别

typedef struct D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS{
    DXGI_FORMAT Format;
    UINT	SampleCount;
    D3D12_MULTISAMPLE_QUALITY_LEVEL_FALGS Flags;
    UINT	NumQualityLevels;
}D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS;

D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
msQualityLevels.Format = mBackBufferFormat;//图像格式
msQualityLevels.SampleCount = 4;//采样次数
msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
msQualityLevels.NumQualityLevels = 0;
ThrowIfFailed(md3dDevice->CheckFeatureSupport(
D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
&msQualityLevels,//填写质量等级
sizeof(msQualityLevels)));
```



不希望使用多重采样，可将采样数量设置为1，并令质量级别为0。



创建交换链缓冲区和深度缓冲区时都要填写DXGI_SAMPLE_DESC结构体。



# 功能级别



Direct3D11引进了功能级别的概念，用**枚举类型D3D_FEATURE_LEVEL**表示。



# DirectX图形基础设施

DirectX图形基础设施(DirectX Graphics Infrastructrue，DXGI，也译作DirectX图形基础设施)。



交换链接口IDXGISwapChain实际上就属于DXGI API。



IDXGIFactory是DXGI中的关键接口，主要用于创建IDXGISwapChain接口以及枚举显示适配器。

适配器用接口IDXGIAdapter来表示。



显示输出用IDXGIOutput接口来表示。

每个适配器与一组显示适配器相关联。



Factory的接口用于获取显示适配器，**由于一个显卡可以和多个显示输出相互关联**，因此用显示适配器的接口来获取显示输出。



每种显示设备都有一系列它所支持的显示模式，可以用**DXGI_MODE_DESC**结构来表示。



一旦确定了显示模式的**具体格式(DXGI_FORMAT)**，就能通过一段代码，获得某个显示输出对此格式

所支持的全部显示模式。



# 初始化Direct3D

1.用D3D12CreateDevice函数创建ID3D12Device接口实例。

2.创建一个ID3D12Fence对象，并查询描述符的大小。

3.检测用户设备对4X MSAA质量级别的支持情况。

4.依次创建命令队列、命令列表分配器和主命令列表。

5.描述并创建交换链。

6.创建应用程序所需要的描述符堆。

7.调整后台缓冲区的大小，并为它创建渲染目标视图。

8.创建深度/模板缓冲区以及与之关联的深度/模板视图。

9.设置视口和裁剪矩形。



## 创建设备

初始化Direct3D，需要先创建Direct3D设备(ID3D12Device)，此设备代表着一个**显示适配器。**



## 创建围栏并获取描述符大小

查询到的描述符大小，要缓存起来。



## 检测对4X MSAA质量级别的支持



## 创建命令队列和命令列表



## 描述并创建交换链

创建交换链，需要填写一份DXGI_SWAP_CHAIN_DESC结构体实例。



## 创建描述符堆

Direct3D 12以ID3D12DescriptorHeap接口表示描述符堆，并用ID3D12Device::CreateDescriptorHeap方法来创建它。



ID3D12DescriptorHeap::GetCPUDescriptorHandleForHeapStart方法来获得描述符堆中的第一个描述符的句柄。



## 创建渲染目标视图



IDXGISwapChain::GetBuffer获取ID3D12Resource资源，然后传入ID3D12Device::CreateRenderTargetView创建渲染目标视图。



## 创建深度/模板缓冲区及其视图

深度缓冲区要创建2D纹理。



纹理是一种2D资源(ID3D12Resource)，要通过填写D3D12_RESOURCE_DESC结构体来描述纹理资源，再用ID3D12Device::CreateCommittedResource方法创建它。



CreateCommittedResource方法将根据我们所提供的属性创建一个资源与一个堆，并把该资源提交到这个堆中。



由默认堆、上传堆、回读堆3种类型。



## 设置视口

填写D3D12_VIEWPORT结构体，使用ID3D12GraphicsCommandList::RSSetViewports来设置视口。



多视口是一种用于对多个渲染目标同事进行渲染的高级渲染技术。



## 设置裁剪矩形



相对于后台缓冲区定义一个裁剪矩形，在此矩形外的像素都将被剔除。



裁剪矩形由类型为RECT的D3D12_RECT结构体定义而成。



要用ID3D12GraphicsCommandList::RSSetScissorRects方法来设置裁剪矩形。



# 计时与动画

性能计时器所用的时间度量单位叫做计数(count)，可调用**QueryPerformanceCounter**函数来获取性能计时器测量的当前时刻值(以计时为单位)。



```C++
//当前时刻值
__int64 currTime;
QueryPerformanceCounter((LARGER_INTEGER*)&currTime);

//每秒有多少计数
__int64 countsPerSec;
QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);

//每个计数需要花费的秒
mSeconsPerCount = 1.0 / (double)countsPerSec;

//计数乘以秒，就可以获取两个间隔时刻所花费的时间
```



mBaseTime，关卡开启的时刻，一个很小的值。

mPausedTime，累积的暂停时间。

mStopTime，停止的总共时间。



# 应用程序框架示例



D3DApp类，提供了创建应用程序主窗口、运行程序消息循环、处理窗口消息以及初始化Direct3D等多种功能的函数。



我们可以根据需求通过实例化一个继承自D3DApp的类型，重写框架的虚函数。



有一些函数介绍：

~D3DApp，析构函数用于释放D3DApp中所用的COM接口对象并刷新命令队列。

析构函数中刷新命令队列的原因是：销毁GPU引用的资源以前，**必须等待GPU处理完队列中的所有命令。**



OnResize，后台缓冲区可以使用IDXGISwapChain::ResizeBuffers方法来调整后台缓冲区的尺寸。

深度/模板缓冲区，则需要在销毁后根据新的工作区大小进行重建。

**渲染目标和深度/模板的视图也应该重新创建。**



D3DApp::CalculateFrameStats相比于FPS更加直观，因为它计算了一帧所需要花费的时间，而不是每秒所渲染的帧数。然后设置到窗口的标题上。



































