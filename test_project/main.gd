extends Node2D

func _ready() -> void:
	test_plugin()

func test_plugin()->void:
	var myClass:Test = Test.new()
	myClass.say_hello()
	print(myClass.my_data)
