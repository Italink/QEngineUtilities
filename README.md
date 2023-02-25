## QEngineUtilities

QEngineUtilities是一个简单的渲染工具库，它包含三个Target：

- **QEngineCore**：渲染架构，包含RHI、FrameGraph、RenderPass、RenderComponent、Asset的简易封装。
- **QEngineEditor**：编辑器套件，包含一些基础属性调整控件，以及基于QtMoc的DetailView。
- **QEngineUtilities**：Lanuch层，对上面两个模块进行组装，例如在`DebugEditor`配置下，会嵌入编辑器，而在`Debug`配置下，就只有Core模块。

它主要用于教学和尝试：

- 强调可读性是第一要素
- 没有细致地追求性能（代码细节上有一些瑕疵，在笔者察觉到的时候已经太晚了，由于精力有限，目前笔者也只能选择妥协，非常抱歉...不过放心，这些影响微乎其微）
- 以渲染为核心，包含少量编辑器架构，不会引入一些会导致代码臃肿的模块，如资产管理，网络，异步，ECS...

一个简单的使用示例如下：

![image-2023022511171977](Resources/image-20230225111719777.png)

![image-20230225112034379](Resources/image-20230225112034379.png)

