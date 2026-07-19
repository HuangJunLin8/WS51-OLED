import os
import tkinter as tk
from tkinter import filedialog, messagebox

def replace_tabs_with_spaces(file_path):
    """将文件中的Tab字符替换为4个空格"""
    try:
        # 读取文件内容
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
        
        # 替换Tab为4个空格
        new_content = content.replace('\t', '    ')
        
        # 写回文件
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(new_content)
        
        return True, f"成功替换所有Tab为4个空格！\n文件：{file_path}"
    except UnicodeDecodeError:
        # 如果UTF-8解码失败，尝试其他编码
        try:
            with open(file_path, 'r', encoding='gbk') as file:
                content = file.read()
            
            new_content = content.replace('\t', '    ')
            
            with open(file_path, 'w', encoding='gbk') as file:
                file.write(new_content)
            
            return True, f"成功替换所有Tab为4个空格！\n文件：{file_path}"
        except Exception as e:
            return False, f"读取文件失败：{str(e)}"
    except Exception as e:
        return False, f"处理文件失败：{str(e)}"

def select_file_and_replace():
    """打开文件选择对话框并处理文件"""
    # 创建隐藏的根窗口
    root = tk.Tk()
    root.withdraw()  # 隐藏主窗口
    
    # 设置初始目录为当前目录
    initial_dir = os.getcwd()
    
    # 弹出文件选择对话框
    file_path = filedialog.askopenfilename(
        title="选择要替换Tab的文件",
        initialdir=initial_dir,
        filetypes=[
            ("所有文件", "*.*"),
            ("文本文件", "*.txt"),
            ("Python文件", "*.py"),
            ("HTML文件", "*.html *.htm"),
            ("CSS文件", "*.css"),
            ("JavaScript文件", "*.js"),
            ("JSON文件", "*.json"),
            ("XML文件", "*.xml")
        ]
    )
    
    # 如果用户取消了选择
    if not file_path:
        messagebox.showinfo("提示", "未选择任何文件，操作已取消")
        return
    
    # 确认操作
    confirm = messagebox.askyesno(
        "确认操作",
        f"即将处理文件：\n{file_path}\n\n确定要将所有Tab字符替换为4个空格吗？"
    )
    
    if not confirm:
        return
    
    # 执行替换
    success, message = replace_tabs_with_spaces(file_path)
    
    # 显示结果
    if success:
        messagebox.showinfo("成功", message)
    else:
        messagebox.showerror("错误", message)

if __name__ == "__main__":
    select_file_and_replace()