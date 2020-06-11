`@Property`是python内置的装饰器。用于限制属性的访问。

在一个属性上面加了该装饰器后，该属性变为只读，相当于`getter`方法。

```python
class Student(object):
    
	@Property
	def name(self):
		return self._name
```

当然也可以设置`setter`方法：

```python
class Student(object):
	
	@Property
	def name(self):
		return self._name
		
    @name.setter
    def name(self, value):
        # 一些检查代码...
    	self._name = value
```

