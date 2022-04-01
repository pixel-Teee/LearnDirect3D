# 命令队列和命令列表

每个GPU都至少维护着一个命令队列(Command Queue)，借助着Direct3D，CPU可以用命令列表(Command List)将命令提交到这个队列中去。**注意，提交过去，不会立马执行。**



命令队列用ID3D12CommandQueue接口来表示。通过填写D3DX12_COMMAND_QUEUE_DESC结构体来描述队列，再调用ID3D12Device::CreateCommandQueue方法创建队列。



ExecuteCommandLists是一种常用的ID3D12CommandQueue接口方法，利用它可将命令列表里的命令

添加到命令队列中去。



ID3D12CommandList有一大堆渲染命令，可以进行调用。



调用ExecuteCommandLists方法才会将命令真正地送入命令队列，供GPU在合适的时机处理。



当命令都被加入到命令列表之后，我们必须调用ID3D12GraphicsCommandList::Close方法来结束命令的记录。



还有一种与命令列表有关的名为ID3D12CommandAlloactor的内存管理类接口。

记录在命令列表中的命令，实际上存储在与之关联的命令分配器上。



命令分配器由ID3D12Device接口来创建。



命令列表同样由ID3D12Device接口来创建。



命令列表有个reset方法，参数是光栅化状态，调用这个，会重制回原先的状态，

命令分配器有个reset方法，相当于vector的clear方法，**清空命令，但是保留容量。**



# CPU与GPU间的同步

强制CPU等待，直到GPU完成所有命令的处理，到达某个指定的围栏点为止。

我们称这种方法为刷新命令队列，可以通过围栏来实现这点。



围栏用ID3D12Fence接口来表示。



# 资源的转换

通过命令列表设置转换资源屏障数组，即可指定资源的转换。

在代码中，资源屏障用D3D12_RESOURCE_BARRIER结构体来表示。



d3dx12.h头文件提供了CD3DX12_RESOURCE_BARRIER，这个结构体继承自D3D12_RESOURCE_BARRIER，

提供了一个辅助函数，用以返回CD3DX12_RESOURCE_BARRIER。



**可以将资源转换屏障看作是一条告知GPU某资源状态正在进行转换的命令。**



p101有多线程的内容和指导。



# 视口和裁剪矩形

视口是宽高以及在窗口的绘制区域，而裁剪矩形则是窗口的裁剪区域。



# 消息处理

WM_ACTIVE，当一个程序被激活或者进入非活动状态的时候便会发送此消息。



```c++
case WM_ACTIVE:
	if(LOWORD(wParam) == WA_INACTIVE)
	{
		mAppPaused = true;
		mTimer.stop();
	}
	else
	{
		mAppPused = false;
		mTimer.Start();
	}
	return 0;
```



处理WM_SIZE的时候，调用D3DApp::OnResize方法。

调用IDXGISwapChain::ResizeBuffers方法改变后台缓冲区的大小，但是深度/模板缓冲区需要在销毁之后，**根据新的窗口尺寸来重新创建。**

渲染目标和深度模板的视图也需要重建。



用户拖动调整栏的操作，会连续发送WM_SIZE消息，我们不希望连续调整缓冲区。

通过处理WM_EXITSIZEMOVE消息可以实现这一点。这条消息会在用户释放调整栏时发送。



```c++
case WM_ENTERSIZEMOVE:
	mAppPaused = true;
	mResizing = true;
	mTimer.Stop();
	return true;
case WM_EXITSIZEMOV:
	mAppPaused = false;
	mResizing = false;
	mTimer.Start();
	OnResize();
	return 0;
```



# 初始化Direct3D演示程序

我们只要从D3DApp中派生出自己的类，实现框架函数并且为此编写特定的代码。



```c++
void InitDirect3DApp::Draw(const GameTimer& gt)
{
	//重复使用记录命令的相关内存
    //只有当与GPU关联的命令列表执行完成时，才能将其重置
    ThrowIfFailed(mDirectCmdListAlloc->Reset());
    
    //在通过ExecuteCommandList方法将某个命令列表加入命令队列后，我们便可以重制该命令列表
    //以此来复用命令列表及其内存
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));
    
    //对资源的状态进行转换，将资源从呈现状态转换为渲染目标状态
    mCommandList->ResourceBarrier(
    1, &CD3DX12_RESOURCE_BARRIER::Transition(
    D3D12_RESOURCE_STATE_PRESENT,
    D3D12_RESOURCE_STATE_RENDER_TARGET
    ));
    
    //设置视口和裁剪矩形
    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);
    
    //清除后台缓冲区和深度缓冲区
    mCommandList->ClearRenderTargetView(
    	CurrentBackBufferView(),
        Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(
    	DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH |
        D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
    
    //指定将要渲染的缓冲区
    mCommandList->OMSetRenderTarget(1, &CurrentBackBufferView(),
    true, &DepthStencilView());
    
    //再次对资源状态进行转换，将资源从渲染目标状态转换为呈现状态
    mCommandList->ResourceBarrier(
    1, &CD3DX12_RESOURCE_BARRIER::Transition(
 	CurrentBackBuffer(),
    D3D12_RESOURCE_STATE_RENDER_TARGET,
    D3D12_RESOURCE_STATE_PRESENT
    ));
    
    //完成命令的记录
    ThrowIfFailed(mCommandList->Close());
    
    //将待执行的命令列表加入命令队列
    ID3D12CommandList* cmdsLists[] = {mCommandList.Get()};
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    
    //交换后台缓冲区和前台缓冲区
    ThrowIfFailed(mSwapChain->Present(0, 0));
    mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;
    
   	FlushCommandQueue();
}
```



ClearDepthStencilView的第二个参数是位标志，表示清除模板还是清除深度。



创建描述符堆的作用是为了给描述符分配内存。

开启调试模式需要启用调试层debugController->EnableDebugLayer，Direct3D会把调试信息发往C++的输出窗口。



颜色向量有它们自己专属的颜色运算，即分量式乘法，想象成光的颜色照射到一个物体表面，

**物体有反射率，得到的结果是反射的占比。**



# 128位颜色



颜色可以使用XMVECTOR，DirectXMath库针对分量式乘法运算提供了下列函数：

```c++
XMVECTOR XM_CALLCONV XMColorModulate(
FXMVECTOR C1,
FXMVECTOR C2
);
```



# 32位颜色

每个分量一个字节，能表示256种颜色，256 * 256 * 256 = 1677216种颜色。



DirectXMath提供了XMCOLOR来表示颜色。



DirectXMath提供了一个获取XMCOLOR类型实例并返回其相应XMVECTOR类型值的函数。

```c++
XMVECTOR XM_CALLCONV PackedVector::XMLoadColor(
	const XMCOLOR* pSource
);
```



DirectXMath提供了一个将XMVECTOR转换至XMCOLOR的函数：

```c++
void XM_CALLCONV PackedVector::XMStoreColor(
XMCOLOR* pDestination,
FXMVECTOR V
);
```



128位颜色通常用于高精度的颜色运算，比如像素着色器中的运算，而32位颜色则用于后台缓冲区，

**因为目前的物理设备还达不到更高的颜色分辨率。**



# 输入装配器阶段

输入装配器阶段会从显存中读取几何数据(顶点和索引)，再将它们装配为几何图元。



装配为几何图元是通过顶点索引的。



# 图元拓扑



在Direct3D中，通过一种名为**顶点缓冲区**的特殊数据结构，将顶点与渲染流水线相互绑定。



将顶点缓冲区内的顶点两两一组解释成线段，还是每3个一组解释成三角形呢？

通过指定**图元拓扑(primitive topology)**来告知Direct3D如何用顶点数据来表示几何图元：

```c++
void ID3D12GraphicsCommandList::IASetPrimitiveTopology(
D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology);
```



指定这个命令之后，后续所有绘制操作都会以这个图元拓扑进行。



LIST列表，表示分离的，而STRIP带，表示相连的。



# 光栅化阶段

光栅化阶段的主要任务是为投影至屏幕上的3D三角形计算出对应的像素颜色。



像素着色器阶段在光栅化阶段之后。















































