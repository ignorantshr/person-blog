mybatis的配置文件

### 属性（properties）

这些属性都是可外部配置且可动态替换的，既可以在典型的 Java 属性文件中配置，亦可通过 properties 元素的子元素来传递。

```xml
<properties resource="org/mybatis/example/config.properties">
  <property name="username" value="dev_user"/>
  <property name="password" value="F2Fa3!33TYyg"/>
</properties>
```

然后其中的属性就可以在整个配置文件中被用来替换需要动态配置的属性值。比如:

```xml
<dataSource type="POOLED">
  <property name="username" value="${username}"/>
  <property name="password" value="${password}"/>
</dataSource>
```

通过`方法参数传递`的属性具有最高优先级，`resource/url 属性`中指定的配置文件次之，最低优先级的是 `properties 属性`中指定的属性。

### 设置（settings）

会改变 MyBatis 的运行时行为。

[详细说明](http://www.mybatis.org/mybatis-3/zh/configuration.html#settings)

### 类型别名（typeAliases）

类型别名是为 Java 类型设置一个短的名字。 它只和 XML 配置有关，存在的意义仅在于用来减少类完全限定名的冗余。

```xml
<typeAliases>
  <typeAlias alias="Author" type="domain.blog.Author"/>
</typeAliases>
```

也可以指定一个包名，MyBatis 会在包名下面搜索需要的 Java Bean：

```xml
<typeAliases>
  <package name="domain.blog"/>
</typeAliases>
```

每一个在包 `domain.blog` 中的 Java Bean，在没有注解的情况下，会使用 Bean 的首字母小写的非限定类名来作为它的别名。 比如 `domain.blog.Author` 的别名为 `author`；若有注解，则别名为其注解值。

```java
@Alias("author")
public class Author {
    ...
}
```

### 类型处理器（typeHandlers）

用类型处理器将获取的值以合适的方式转换成 Java 类型。

### 环境配置（environments）

**尽管可以配置多个环境，但每个 SqlSessionFactory 实例只能选择一种环境。**

每个数据库对应一个环境。

**事务管理器（transactionManager）**

- JDBC – 这个配置就是直接使用了 JDBC 的提交和回滚设置，它依赖于从数据源得到的连接来管理事务作用域。
- MANAGED – 这个配置几乎没做什么。它从来不提交或回滚一个连接，而是让容器来管理事务的整个生命周期（比如 JEE 应用服务器的上下文）。

如果你正在使用 Spring + MyBatis，则没有必要配置事务管理器， 因为 Spring 模块会使用自带的管理器来覆盖前面的配置。

**数据源（dataSource）**

类型：

- **UNPOOLED** 

    > 这个数据源的实现只是每次被请求时打开和关闭连接。

- **POOLED**

    > 这种数据源的实现利用“池”的概念将 JDBC 连接对象组织起来，避免了创建新的连接实例时所必需的初始化和认证时间。

- **JNDI**

    > 这个数据源的实现是为了能在如 EJB 或应用服务器这类容器中使用，容器可以集中或在外部配置数据源，然后放置一个 JNDI 上下文的引用。

### 映射器（mappers）

你可以使用相对于类路径的资源引用， 或完全限定资源定位符（包括 `file:///` 的 URL），或类名和包名等。例如：

```xml
<mappers>
    <!-- 使用相对于类路径的资源引用 -->
  <mapper resource="org/mybatis/builder/AuthorMapper.xml"/>
    <!-- 使用完全限定资源定位符（URL） -->
  <mapper url="file:///var/mappers/AuthorMapper.xml"/>
	<!-- 使用映射器接口实现类的完全限定类名 -->
  <mapper class="org.mybatis.builder.AuthorMapper"/>
    <!-- 将包内的映射器接口实现全部注册为映射器 -->
  <package name="org.mybatis.builder"/>
</mappers>
```

