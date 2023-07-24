# Hypereal2OpenVR

此动态库可以将SteamVR的调用转为Hypereal的API调用。

代码结构参考自[Relativty Driver](https://github.com/relativty/Relativty)，经过完全重构。

## 特别鸣谢

感谢老板 `C*g（666￥）`的宇宙飞船，祝老板事事顺心，马到成功，工作666，生活666 /拱手

感谢老板 `Max🐯（100￥）`的火箭，祝老板身体健康，工作顺利，心想事成，万事如意

感谢 `星凌（10￥）`，`三木（10￥）`，`宇浩喵（12￥）`，`西街甜品店（20￥）`，`秦广告（18.88￥）`，`洋葱（12￥）`，`依棠（9.9￥）`赞助的午饭，

感谢 `L（8￥）`，`hengnio（6￥）`，`Maple Rin（5￥）`，`世鹰（5￥）`，赞助的奶茶，

感谢 `这是为了活下去的挣扎（3￥）`，`三甲断大（3￥）`，`藤 原 拓 海（4￥）`赞助的快乐水。

---

魔改了部分Hypereal OpenVR Driver的字节码以阻止hvr加载手柄。

安装方法：

（版本<=0.5.1）

①解压至 `steamvr/driver`目录下

②在steamvr设置内打开高级设置，启动项管理全部勾选

③修改steam/config/steamvr.setting文件，在 `"steamvr":{`行的下一行添加内容为 `"activateMultipleDrivers" : true,`的新行

（版本>=1.0)

①解压至 `steamvr/driver`路径下，可能需要在 `设置->启动/关闭->管理加载项`中手动开启对应的启动项。

感谢小凌瞎玩@bilibili制作的[视频安装教程](http://b23.tv/eUQDddw)

亦有本人简单录制的[安装方法](https://www.bilibili.com/video/bv14Y4y1n7Q9)(求三连)

[蓝奏云](https://wwt.lanzoub.com/b0d48942b)(仅同步最新驱动文件) 密码:6hpk

[源代码](https://github.com/lixiangwuxian/Hypereal2OpenVR)

---

### Changelog:

#### 2022.7.1 - v0.4

- 添加手柄连接状态检测；
- 添加手柄电量检测；
- 添加错误提示（依官方SDK原样提供）；
- 无需点击“是”便可以正常使用手柄；

#### 2022.8.19 - v0.5

- 稳定呼出SteamVR菜单；
- 正常显示第三个摄像头；
- ~~驱动启动后自动结束Hy菜单界面进程（bkdrop.exe）；~~ 代码实现有误，待修正

#### 2022.8.27 - v0.5.1

- 驱动启动后自动结束Hy菜单界面进程（bkdrop.exe）（已修正）
- 手柄图标现在与Hypereal官方SteamVR驱动图标相同

#### 2022.9.22 - v1.0

- 安装前请删除旧版本的驱动文件
- 修复震动（理论效果与旧版steamvr相同）
- 实现IVRVirtualDisplay接口，但是畸变和坐标系待进一步修正，**可能很晕**
- 摄像头不显示，但是使用无任何影响

#### 2022.10.27 - v2.0

- 修复画面扭曲
- 因为暂不知IVRVirtualDisplay中垂直同步功能如何实现，存在转头时画面抖动情况
- 摄像头不显示（未实现基站类），但是追踪使用上无任何影响。

---

## 手动编译

需要下载安装VS2022，可能还需要额外安装Windows Kit。打开  `SteamVR4Hypereal.sln` ，按下 `CTRL`+`B` 即可编译。

生成目录为 `revive_hypereal\bin\win64`

---

PS: 在这里下载SDK:[link](https://bugzilla.mozilla.org/page.cgi?id=splinter.html&ignore=&bug=1378630&attachment=8883821).

感谢Junwen Liang于bugzilla上传的SDK和驱动样例.
