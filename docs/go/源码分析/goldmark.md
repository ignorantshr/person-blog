goldmark

## 思路

### 生成 markdown ast 语法树

流程：

1. 一行一行地读取文件，以空行为分隔符把空行之间的内容作为一个 block，block 可以嵌套，每个 block 就代表了一个 ast 的节点

2. **不断寻找有内容的行**。首先不断的跳过空白行，找到第一处不为空行的位置，然后执行 *openBlocks*，向下推进一行，处理这些 opened-blocks

3. **处理 opened-blocks**。循环处理
   
   1. 判断 opened-blocks 的数量，为0时跳出循环。
   
   2. 遍历 opened-blocks，对每个 block 进行以下操作：
      
      判断是否为段落节点 paragraph-block ，真{ 使用 block.Parser.Continue 继续判断是否可以继续，真{ 该添加子节点的话执行 *openBlocks*，打断此次遍历 }，继续遍历 opened-blocks }。
      
      因为在找到 block 后或者循环时已经推进到下一行了，对新行执行 *openBlocks* ，若结果不是段落继续 paragraphContinuation { 则说明一些 blocks 已经解析结束了，关闭它们 }，打断此次遍历。
   
   3. 推进一行

补充说明：

1. **openBlocks**。提取出一行，判断并构造具体类型的 block，追加到 context 的 opened-blocks 列表中（依情况而定有时也会关闭列表中的 block），如果还有 child-block，也将其加入到列表，并将新构造的 block 作为其父节点。同时返回本次尝试的结果，以作为调用方下次行为的依据

2. 在开始转换的时候会加载一次配置，比如 parser、transformer，每个解析器列表都有优先级，遇到第一个能够解析的 parser 就会使用它来创建节点

3. 文件读取完毕后 ast 语法树也即构建完成，每个节点都是在 openBlocks 中构建的

#### 数据结构

```go
type Block struct {
    // Node is a BlockNode.
    Node ast.Node
    // Parser is a BlockParser.
    Parser BlockParser
}
```

#### Parser

##### openBlocks

打开 blocks

```go
func (p *parser) openBlocks(parent ast.Node, blankLine bool, reader text.Reader, pc Context) blockOpenResult
```

`parent ast.Node` ast语法树结点

`pc Context` 解析时的上下文信息

`result` = noBlocksOpened

`lastBlock`最后一个正在解析的 block，openedBlock

`continuable` = ast.IsParagraph(lastBlock.Node) 是否为段落节点

【1】`line`提取出一行内容

计算并保存缩进信息到 pc

若此行结束，`goto continuable` 返回解析状态【2】

否则继续往下走

`bps` 获取当前位置字节，通过相应的触发字节查找对应的 block parser list，默认是遇到任意字节都会触发的 free block parser list

若 bps 为空 【2】

遍历 bps，找到第一个合适的 `bp`，构建 ast.Node 节点 {

`lastBlock` 获取最后一个 openedblock

`last` lastBlock.Node

`node, state` 尝试使用 bp 解析并创建一个新的 ast.Node ，返回状态（Continue,Close,HasChildren,NoChildren,RequireParagraph ）

若 state == RequireParagraph 是段落，并且 last == parent.LastChild()

    关闭 last

    更新 pc 的 openedBlockList

    把 last 转换成段落节点，若 last 的父节点为空，继续尝试打开 block Node【1】

}

node 根据 blankLine 设置前一行的空行状态

若 last 且 last 的父节点非空，关闭 pc.openedBlockList 中最后一个 block，同时转换段落

添加 node 到 parent 的孩子节点列表

更新 result = newBlocksOpened

构造新的 Block{node, bp} ，添加到 pc.openedBlockList

若 state == HasChildren , parent = node, 继续尝试打开新的 block【1】

否则 break，因为此时没有孩子节点了，不需要继续了

}

【2】if result == noBlocksOpened && continuable，使用 lastBlock.Parser.Continue 判断是否可以继续，可以的话 更新 result = paragraphContinuation

return result

##### parseBlocks

解析 blocks 

```go
func (p *parser) parseBlocks(parent ast.Node, reader text.Reader, pc Context)
```

把 pc 的 opened block list 置空

初始化 `blankLines` ，isBlank := false

for { // 处理被空行分隔的 blocks

`lines` 从游标位置开始统计的空白行数量

`lineNum` 当前行的行号

若 找到了空白行{

    blankLines 置空

    `l := len(pc.OpenedBlocks())`打开的 blocks 的数量

    blankLines 填入 l 个空行，每个空行的状态是 `lineStat{lineNum:lineNum-1,level:0-l,isBlank:true}`

}

`isBlank` 前一行是否空白行

`p.openBlocks` 打开 blocks 

reader 的游标推进一行

for { // 一行一行地处理 openedBlocks

    `openedBlocks` 打开的 blocks

    `lastIndex` 最后一个 block 的位置索引

    for i := 0; i < l; i++ { // 遍历 openedBlocks

        `be := openedBlocks[i]`

        `line` 取出当前行内容

        若 line == nil ，关闭最后的 block，推进 reader 游标，return

        `lineNum` 当前行位置

        blankLines 添加一行 lineStat{lineNum, i, util.IsBlank(line)}

        // p.openBlocks 决定段落继续与否，这里不做处理

        if !ast.IsParagraph(be.Node) {            

            若 state&Continue != 0 {

                若有子节点并且达到了 lastIndex, 尝试打开新的子节点 {

                    p.openBlocks(be.Node, isBlank, reader, pc)

                    break

                }

                continue，跳过下面的处理

            }

        }

        `thisParent = i==0 ？ parent : openedBlocks[i-1].Node` 

        `lastNode` 最后一个 block 的 Node

        `result := p.openBlocks(thisParent, isBlank, reader, pc)` 打开父节点的 blocks

        若 `result != paragraphContinuation` {关闭最后一个 block}

        break，跳出此次 opened block list 遍历

    }

    reader 游标推进一行

}

}
