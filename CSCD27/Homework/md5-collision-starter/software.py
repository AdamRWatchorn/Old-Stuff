# -*- coding: utf-8 -*-
bits = """
                             �xuF��,�]2ބ�
�4Gu��$Ef{U)��f��Z(����w�U�y�%ޗ�7݀	�F���9�BL)�w�����q"�$Z+.��Zv9�F�eȃ��
ۀ�#P����(j��O���@]	�J
"""
from hashlib import sha256

good = "a2405517ac382b7775f5036d801888045a1deff87008943be9aebfeb0b6b9492"

if(sha256(bits).hexdigest() == good):
    print("I am good!")
else:
    print("I am evil!")
