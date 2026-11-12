# LabelGen
Program to read Excel file with list products and generate pdf-file with labels on each product (optional with QR-code).
------------------------------------------------------------------------------------------------------------------------
This is pet-project for education and for portfolio.
===================================================

Program "LabelGen" read Excel files (*.xls, *.xlsx) and generate label of the product using rules:
1. first row of Excel file must content attributes of products (for example, "name", "brend", "mass", "price" and etc.)
2. second and next rows of Excel file should content values of these attributes (for example, "milk", "MyMilkBrend", "500 gr", "1$" and etc)
For one row with values will be generated one label.

Program LabelGen provide for user next actions:
1. delete some attribute
2. change order of attributes in result label
3. change font for selected attribute
4. generate QR-code with attributes and values and place it below or right from text

Resulting labels write in pdf-file on one or several pages A4, combining labels in one page A4 as many as possible.

######################################
In program is used next opensource libraries from github:
1. QXlsx
2. Libxls (Evan Miller)
3. qcoregen (Project Nayuki)
