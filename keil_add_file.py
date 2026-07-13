import os
import glob
import xml.etree.ElementTree as ET
import argparse

from multiprocessing import Event

def indent(elem, level=0):
    """ Helper function to indent the XML for pretty printing. """
    i = "\n" + level * "    "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "    "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            indent(elem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i
        if not elem.tail:
            elem.tail = "\n"

def add_files_to_group(uvprojx_file_path, mode,folder_path, group_name_target):
    # 改变文件扩展名到 .xml (支持 .uvproj 和 .uvprojx)
    base, ext = os.path.splitext(uvprojx_file_path)
    if ext not in ('.uvprojx', '.uvproj'):
        print("工程文件扩展名不正确，需要 .uvproj 或 .uvprojx")
        return
    
    xml_path = base + '.xml'
    os.rename(uvprojx_file_path, xml_path)

    try:
        #解析XML文件
        tree = ET.parse(xml_path)
        #获取根节点
        root = tree.getroot()

        if mode == 0 or mode == 1:
            # 找到指定GroupName的Group节点
            target_group = None
            for group in root.findall('.//Group'):
                group_name = group.find('GroupName')
                if group_name is not None and group_name.text == group_name_target:
                    target_group = group
                    break

            if target_group is None:
                print(f"未发现 '{group_name_target}' 分组，请先创建分组后再尝试")
                # 将文件扩展名改回 .uvprojx
                os.rename(xml_path, uvprojx_file_path)
                return

            # 找到目标 Group 节点下的 Files 节点，如果不存在则创建一个
            files_node = target_group.find('Files')
            if files_node is None:
                files_node = ET.SubElement(target_group, 'Files')
                                
        #寻找头文件分组
        if mode == 0 or mode == 2:
            print("寻找头文件分组......")
            heard_inc = None

            # 先尝试 MDK/ARM 格式 (Cads), 再尝试 C51 格式 (C51)
            for compiler_tag in ('.//Cads', './/C51', './/Ax51'):
                target_header = root.find(compiler_tag)
                if target_header is not None:
                    vc = target_header.find('VariousControls')
                    if vc is not None:
                        hi = vc.find('IncludePath')
                        if hi is not None:
                            heard_inc = hi
                            print(f"找到头文件分组 ({compiler_tag})")
                            break

            if heard_inc is None:
                print("未发现头文件分组 (已尝试 Cads/C51/Ax51)")
                return
                       
        
        
        #下面没有节点
        if mode == 0 or mode == 1:
            creat_dot = 0 #是否需要创建节点标志,如果有重复则跳过 
            init_creat = 0
            file_init = files_node.find('File')
            if file_init == None:
                creat_dot = 1
                init_creat = 1
                print("初始节点为空需要创建节点")
  
        # 遍历指定文件夹，查找所有 .c 文件  
        if mode == 0 or mode == 2:
            #print(heard_inc.text)  
            heard_data = heard_inc.text + ";"   #末尾需要先加一个分号   
            
        for subdir, _, files in os.walk(folder_path):
            #.h路径
            if mode == 0 or mode == 2:
                dir_path = os.path.relpath(subdir, start=os.path.dirname(xml_path))
                if dir_path in heard_inc.text:
                    print("需要添加的头文件路径已存在本次跳过")
                else:
                    heard_data = heard_data + dir_path + ";"
                    
            #.c添加到分组
            if mode == 0 or mode == 1:
                for file in files:     
                    if file.endswith('.c'):
                        # 计算相对路径
                        file_path = os.path.relpath(os.path.join(subdir, file), start=os.path.dirname(xml_path))
                        #print("路径",file_path)
                        file_check = files_node.findall('File')
                        if init_creat == 0:
                            #print("长度",len(file_check))
                            #遍历当前分组下的节点,检测是否已经包含了该路径,如果有直接跳过
                            for i in range(len(file_check)):
                                if file_path in file_check[i].find("FilePath").text:
                                    print("节点已存在本次跳过")
                                    creat_dot = 0
                                    break
                                else: 
                                    if i == len(file_check) - 1:
                                        creat_dot = 1
                                        print("节点不存在,创建节点")
                                    else:
                                       creat_dot = 0
                                    continue    
                        if creat_dot == 1:
                            # 创建 File 节点并添加到 Files 节点下
                            file_node = ET.SubElement(files_node, 'File')
                            file_name_node = ET.SubElement(file_node, 'FileName')
                            file_name_node.text = file
                            file_type_node = ET.SubElement(file_node, 'FileType')
                            file_type_node.text = '1'  # .c 文件类型都为 1

                            file_path_node = ET.SubElement(file_node, 'FilePath')
                            file_path_node.text = file_path
                            creat_dot = 0
                            init_creat = 0
                            
        if mode == 0 or mode == 2:
            heard_data = heard_data.rstrip(";") #移除最后一个多加的;
            heard_inc.text = heard_data
            #print(heard_inc.text)                    
                  
        # 格式化 XML
        indent(root)

        # 保存修改后的 XML 文件
        tree.write(xml_path, encoding='utf-8', xml_declaration=True)
        print("已完成")

    except ET.ParseError as e:
        print(f"ParseError: {e}")
        with open(xml_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()
            start = max(0, e.position[0] - 5)
            end = min(len(lines), e.position[0] + 5)
            print("Context around the error:")
            for i in range(start, end):
                print(f"{i+1}: {lines[i].strip()}")

    finally:
        # 将文件扩展名改回原扩展名
        os.rename(xml_path, base + ext)

#寻找工程文件
def find_uvprojx_file():
    uvprojx_files = glob.glob("*.uvprojx") + glob.glob("*.uvproj")
    # 去重（防止同一个文件被匹配两次）
    uvprojx_files = list(set(uvprojx_files))
    if not uvprojx_files:
        print("未找到工程文件(.uvproj/.uvprojx),请把此文件放在keil工程目录下")
        return None
    elif len(uvprojx_files) > 1:
        print("在当前目录中找到多个工程文件：")
        for i, file in enumerate(uvprojx_files, start=1):
            print(f"{i}. {file}")
        print("请确保目录中只有一个.uvproj或.uvprojx文件")
        return None
    else:
        return uvprojx_files[0]

if __name__ == "__main__":
    print("keil一键添加文件和头文件路径脚本\n\
    需放在keil工程同级目录下\n\
    参数格式,参数用空格隔开\n\
    默认模式:0\n\
    默认路径:\"../../../external/lvgl\"\n\
    默认分组:\"lvgl\"\n\
    1.添加模式 0:全部添加(.c文件全添加到分组.h文件夹加入include路径里) 1:只添加.c文件到分组 2:只添加.h文件夹到include里\n\
    2.要添加的文件夹路径,请使用相对路径\n\
    3.要添加的分组名称,如果没有分组需要先去keil手动添加分组\n")
    
    param = input("请输入参数:")
    
    if param:
        #print(param)
        args = param.split()
        args[0] = int(args[0])
        print(args)
    else:
        args = [0,"../../../external/lvgl","lvgl"]
        print("使用默认参数:",args)
    uvprojx_file_path = find_uvprojx_file()
    if uvprojx_file_path:
        add_files_to_group(uvprojx_file_path, args[0],args[1],args[2])

    event = Event()
    event.wait()