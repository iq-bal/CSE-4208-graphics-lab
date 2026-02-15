
import PyPDF2

pdf_path = "/Users/iqbalmahamud/project/graphics/project/graphics_project_idea.pdf"
output_path = "/Users/iqbalmahamud/project/graphics/project_requirements.txt"

try:
    with open(pdf_path, "rb") as file:
        reader = PyPDF2.PdfReader(file)
        text = ""
        for page in reader.pages:
            text += page.extract_text() + "\n"
    
    with open(output_path, "w") as f:
        f.write(text)
    print(f"Successfully wrote to {output_path}")

except Exception as e:
    print(f"Error: {e}")
