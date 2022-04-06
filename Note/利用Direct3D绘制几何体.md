# 顶点与输入布局

定义了顶点结构体之后，需要向Direct3D提供该顶点结构体的描述，使它了解应该怎样来处理结构体中的每个成员。称为**输入布局描述(input layout description)。**



用结构体D3D12_INPUT_LAYOUT_DESC来表示。

```c++
typedef struct D3D12_INPUT_LAYOUT_DESC
{
	const D3D12_INPUT_ELEMENT_DESC* pInputElementDesc;
	UINT NumElements;
}D3D12_INPUT_LAYOUT_DESC;
```



D3D12_INPUT_ELEMENT_DESC描述了顶点结构体中对应的成员：

```c++
typedef struct D3D12_INPUT_ELEMENT_DESC
{
	LPCSTR SemanticName;//语义名
	UINT SemanticIndex;//语义索引
	DXGI_FORMAT Format;//顶点元素格式
	UINT InputSlot;//输入槽，暂时为0，练习里面有介绍
	UINT AlignedByteOffset;//顶点在顶点结构体中的偏移量
	D3D12_INPUT_CLASSIFICATION InputSlotCass;//实例化用
	UINT InstanceDataStepRate;//实例化用
}D3D12_INPUT_ELEMENT_DESC;
```



# 顶点缓冲区

为了使GPU可以访问顶点数组，需要把它放置在称为缓冲区的GPU资源(ID3D12Resource)里面。



通过填写D3D12_RESOURCE_DESC结构体来描述缓冲区资源，接着再调用ID3D12Device::CreateCommittedResource方法创建ID3D12Resource资源。



Direct3D 12提供了一个C++包装类CD3DX12_RESOURCE_DESC，它派生自D3D12_RESOURCE_DESC结构体，

带有多种便于使用的构造函数以及方法。

```c++
static inline CD3DX12_RESOURCE_DESC Buffer(
UINT64 width,
D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
UINT64 alignment = 0
)
{
	return CD3D12_RESOURCE_DESC(D3D12_RESOURCE_DIMENSION_BUFFER,
	alignment, width, 1, 1, 1,
	DXGI_FORMAT_UNKNOWN, 1, 0,
	D3D12_TEXTURE_LAYOUT_ROW_MAJOR, flags);
}
```



width表示缓冲区所占的字节数。

Buffer这个函数，用来返回一个描述创建缓冲区的描述结构体。



缓冲区用D3D12_RESOURCE_DIMENSION_BUFFER类型来表示，而2D纹理则以D3D12_RESOURCE_DIMENSION_TEXTURE2D来表示。



顶点缓冲区资源这种类型是默认堆，除了创建顶点缓冲区资源本身以外，

我们还需要用D3D12_HEAP_TYPE_UPLOAD这种堆类型，

创建一个处于中介位置的**上传缓冲区资源。**



```c++
Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefaultBuffer
(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& UpoladBuffer
)
{
	//默认堆
	ComPtr<ID3D12Resource> defaultBuffer;
	
	//创建实际的默认缓冲区资源
	ThrowIfFailed(device->CreateCommittedResource(
	&CD3DX12_RESOUCE_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	D3D12_HEAP_FLAG_NONE,
	&CD3D12_RESOURCE_DESC::Buffer(byteSize),
	D3D12_RESOURCE_STATE_COMMON,//资源状态
	nullptr,
	IID_PPV_ARGS(defaultBuffer.GetAddressOf())
	));

	//创建中介堆
	ThrowIfFailed(device->CreateCommittedResource(
	&CD3DX12_RESOURCE_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
	D3D12_HEAP_FLAG_NONE,
	&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
	D3D12_RESOURCE_STATE_GENERIC_READ,//这里
	nullptr,
	IID_PPV_ARGS(uploadBuffer.GetAddressOf())
	));
	
	//描述我们希望复制到默认缓冲区的数据
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;//这里，关键，从内存拷贝数据
	subResourceData.RowPitch = byteSize;//要复制的字节数
	subResourceData.SlicePitch = subResourceData.RowPitch;
	
	//将数据复制到默认缓冲区资源的流程
	//UpdateSubresources辅助函数会先将数据从CPU端的内存中复制到位于中介位置的上传堆里，
	//接着通过调用ID3D12CommandList::CopySubresourceRegion函数，把上传堆内的数据复制到
	//mBuffer中
	cmdList->ResourceBarrier(1,
    &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COMMON,
    D3D12_RESOURCE_STATE_COPY_DST));
    
    //关键
    UpdateSubresources<1>(cmdList,
    defaultBuffer.Get(), uploadBuffer.Get(),
    0, 0, 1, &subResourceData);
    
    cmdList->ResourceBarrier(1,
    &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
    D3D12_RESOURCE_STATE_COPY_DEST
    D3D12_RESOURCE_STATE_GENERIC_READ));
    
    //调用完上述函数后，必须保证uploadbuffer还存在，因为命令列表的命令还未执行，
    //等执行完毕后，再释放
    return defaultBuffer;
}
```



```c++
typedef struct D3D12_SUBRESOURCE_DATA
{
	const void* data,//指向内存块的数据
	LONG_PTR RowPitch,
	LONG_PTR SlicePitch
}D3D12_SUBRESOURCE_DATA;
```

![image-20220406162959502](../Image/image-20220406162959502.png)

VertexBufferGPU：默认堆

VertexBufferUploader：上传堆



为了将顶点缓冲区绑定到渲染流水线上，需要给这种资源创建一个顶点缓冲区视图，**和RTV一样，**

无须为顶点缓冲区视图创建描述符堆。顶点缓冲区视图是由D3D12_VERTEX_BUFFER_VIEW结构体来表示的。



```c++
typedef struct D3D12_VERTEX_BUFFER_VIEW{
	D3D12_GPU_VIRTUAL_ADDRESS BufferLocation,//buffer的虚拟地址
	UINT SizeInBytes,//顶点缓冲区大小
	UINT StrideInBytes//一个顶点的字节数
}D3D12_VERTEX_BUFFER_VIEW;
```



第一个参数，可以通过ID3D12Resource::GetGPUVirtualAddress方法来获得此地址。



在顶点缓冲区及其对应视图创建完成后，便可以将它与渲染流水线上的一个**输入槽**相互绑定。

```c++
void ID3D12GraphicsCommandList::IASetVertexBuffers(
UINT StartSlot,
UINT NumView,
const D3D12_VERTEX_BUFFER_VIEW* pViews
);
```

第一个参数开始的槽位会和第三个参数依依绑定。



**//后续有个小作业，会使用多个槽**



最后一步是通过ID3D12GraphicsCommandList::DrawInstanced方法来绘制顶点。

```c++
void ID3D12GraphicsCommandList::DrawInstanced(
UINT VertexCountPerInstance,//每个实例要绘制的顶点数量
UINT InstanceCount,//暂时为1
UINT StartVertexLocation,//指定顶点缓冲区内第一个被绘制顶点的索引
UINT StartInstanceLocation//暂时为0，实例化用
);
```



我们还要指定一下图元拓扑：

```c++
cmdList->IASetPrimitiveTopology(D3D_PRIMTIVE_TOPLOGY_TRAINGLELIST);
```



# 索引缓冲区

也是ID3D12Resource来存放索引数据。



为了使索引缓冲区与渲染流水线相绑定，需要给索引缓冲区资源创建一个索引缓冲区视图(index buffer view)。



索引缓冲区视图由结构体D3D12_INDEX_BUFFER_VIEW来表示。

```C++
typedef struct D3D12_INDEX_BUFFER_VIEW
{
	D3D12_GPU_VIRTUAL_ADDRESS BufferLocation,
	UINT SizeInBytes,
	DXGI_FORMAT Format
}D3D12_INDEX_BUFFER_VIEW;
```



```c++
mCommandList->IASetIndexBuffer(&ibv);
```



使用DrawIndexedInstanced来代替DrawInstanced方法进行绘制。



# 顶点着色器示例

```c++
cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
};

void VS(float3 iPosL : POSITION,
		float4 iColor : COLOR,
		out float4 oPosH : SV_POSITION,
		out float4 oColor : COLOR)
{
	//把顶点变换到齐次裁剪空间
	oPosH = mul(float(iPosL, 1.0f), gWorldViewProj);
	
	//直接将顶点的颜色信息输出到像素着色器
	oColor = iColor;
}
```



注意关键字，out是输出参数。在HLSL中，所有的函数都是内联函数。



矩阵变量gWorldViewProj存于**常量缓冲区**内。



顶点输入布局描述必须和顶点着色器中的一致，Direct3D会验证两者是否一致，

**创建ID3D12PipelineState的时候，会进行验证。**



# 像素着色器示例

光栅化处理期间会对顶点着色器输出的顶点属性**进行插值**。随后，再将这些插值数据传至像素着色器中作为它的输入。



像素着色器输入的是像素片段，输出的是像素。



像素片段会进行竞争，比如执行深度测试，调用discard，并不会真正地写入像素。



```c++
float4 PS(float4 posH : SV_POSITION, float4 color : COLOR) : SV_Target
{
	return color;
}
```



**SV_Target，表示返回值的类型应当与渲染目标格式相互匹配。**



# 常量缓冲区



常量缓冲区是一种GPU资源，其数据内容可供着色器程序所引用。



常量缓冲区通常由CPU每帧更新一次。

我们需要把常量缓冲区创建到一个上传堆而非默认堆中，**注意，这里，顶点数据是创建到默认堆的。**

常量缓冲区的大小必须为硬件最小分配空间(256B)的整数倍。



CalcConstantBufferByteSize用来计算常量缓冲区的大小。



HLSL会自行进行填充。



# 更新常量缓冲区

常量缓冲区是用D3D12_HEAP_TYPE_UPLOAD这种堆类型来创建的，**我们能通过CPU为常量缓冲区资源更新数据。**



首先要获得指向欲更新资源数据的指针，用Map方法来做到这一点：

```c++
ComPtr<ID3D12Resource> mUploadBuffer;
BYTE* mMappedData = nullptr;
mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData));
```



第一个参数是子资源索引，指定了欲映射的子资源。对于缓冲区来说，它自己本身就是子资源。

第二个参数是可选项，指向D3D12_RANGE结构体的指针，此结构体描述了内存的映射范围，若将该参数指定为空指针，则对整个资源进行映射。



```c++
memcpy(&mMappedData, &data, dataSizeInBytes);
```



当常量缓冲区更新完毕之后，我们应在释放映射内存之前对其进行unmap操作。

```c++
if(mUploadBuffer != nullptr)
	mUploadBuffer->Unmap(0, nullptr);
	
mMappedData = nullptr;
```



## 上传缓冲区辅助函数



UploadBuffer.h封装了上传缓冲区，实现了上传缓冲区的构造与析构，处理资源的映射和取消映射关系，

提供了**CopyData**方法来更新缓冲区内的特定元素。



//200























