import sys
import os
import charset_normalizer
from charset_normalizer import from_bytes
import tempfile
import shutil


def detect_file_encoding(file_path, threshold: float = 0.0):
    """
    使用 charset-normalizer 检测文件编码，并打印所有候选结果。

    :param file_path: 文件路径
    :param threshold: 置信度阈值，低于此值则返回 None
    :return: (encoding, confidence) 或 (None, 0.0)
    """
    if not os.path.exists(file_path) or not os.path.isfile(file_path):
        print(f"[错误] {file_path} 不存在或不是文件")
        return None, 0.0

    try:
        with open(file_path, "rb") as f:
            raw_data = f.read()

        results = from_bytes(raw_data)

        if not results:
            print("[提示] 未检测到任何编码候选")
            return None, 0.0

        print(f"[候选编码结果] {file_path}:")
        for match in results:
            print(f"  - {match.encoding:<15} 置信度={match.fingerprint:.2f} 语言={match.language}")

        best = results.best()
        if best and best.fingerprint > 0.0 and best.fingerprint >= threshold:
            return best.encoding, best.fingerprint
        else:
            print("[提示] 检测到的编码置信度过低")
            return None, 0.0

    except Exception as e:
        print(f"[错误] {file_path}: {e}")
        return None, 0.0

def process_file(file_path, target_encoding="utf-8"):
    """
    检测输入文件的编码，并将其内容转换为目标编码格式。

    :param file_path: 输入文件路径
    :param target_encoding: 目标文件编码格式，默认为 utf-8
    :return: (转换结果, 错误原因)
    """
    if not os.path.exists(file_path):
        return False, f"{file_path}: 文件不存在"

    if not os.path.isfile(file_path):
        return False, f"{file_path}: 路径不是一个文件"

    source_encoding = ""
    try:
        # 检测文件编码
        with open(file_path, "rb") as file:
            raw_data = file.read()
            detected = charset_normalizer.detect(raw_data)
            source_encoding = detected.get("encoding")
            print(source_encoding)

        if not source_encoding:
            return False, f"{file_path}: 无法检测文件编码"

        # 使用检测到的编码读取文件内容
        try:
            with open(file_path, "r", encoding=source_encoding) as file:
                content = file.read()
        except UnicodeDecodeError:
            return False, f"{file_path}: 无法使用检测到的编码 {source_encoding} 解码文件"

        # 写入到临时文件，再覆盖原文件
        dir_name = os.path.dirname(file_path)
        with tempfile.NamedTemporaryFile("w", encoding=target_encoding, delete=False, dir=dir_name) as tmp_file:
            tmp_file.write(content)
            temp_file_path = tmp_file.name

        shutil.move(temp_file_path, file_path)  # 原子替换

        return True, ""

    except FileNotFoundError:
        return False, f"{file_path}: 文件未找到"
    except PermissionError:
        return False, f"{file_path}: 没有权限访问文件"
    except UnicodeDecodeError:
            return False, f"{file_path}: 无法检测dao 文件编码"
    except Exception as e:
        return False, f"{file_path}: 未知错误: {str(e)}"

def main():
    while True:
        try:
            # 从标准输入读取数据
            # change,path1,path2,path3,.....
            input_data = sys.stdin.readline().strip()
            if not input_data:
                break
            parts = input_data.split(',')
            if len(parts) < 1:
                continue
            command = parts[0]
            if command == "change":
                encodee = parts[1]
                filePaths = parts[2:]
                # 错误原因组合成以,分割的字符串, 统计成功的文件数量和失败的文件数量. 返回结果为 change_return,成功数量,失败数量,错误原因字符串
                success_count = 0
                failure_count = 0
                error_reasons = []
                for file_path in filePaths:
                    result, reason = process_file(file_path,encodee)
                    if result:
                        success_count += 1
                    else:
                        failure_count += 1
                        error_reasons.append(reason)
                error_reasons_str = ','.join(error_reasons)
                output = f"change_return,{success_count},{failure_count},{error_reasons_str}\n"
                sys.stdout.write(output)
                sys.stdout.flush()
            if command == "test":
                file_path = parts[1]
                content = detect_file_encoding(file_path)
                output = f"test_return,{content}\n"
                sys.stdout.write(output)
                sys.stdout.flush()
            else:
                sys.stderr.write(f"Unknown command: {command}\n")
                sys.stderr.flush()
        except Exception as e:
            sys.stderr.write(f"Error: {str(e)}\n")
            sys.stderr.flush()

if __name__ == "__main__":
    main()