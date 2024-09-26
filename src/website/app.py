from flask import Flask, request, render_template, redirect, url_for, send_from_directory
import os

app = Flask(__name__)


UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

@app.route('/')
def upload_form():
    return render_template('upload.html')

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return redirect(request.url)
    file = request.files['file']
    if file.filename == '':
        return redirect(request.url)
    
    if file and file.filename.endswith('.hex'):
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], file.filename)
        file.save(filepath)
        return f'File uploaded successfully: {file.filename}'
    else:
        return 'Invalid file format. Please upload a .hex file.'


@app.route('/uploads')
def list_files():
    files = os.listdir(app.config['UPLOAD_FOLDER'])
    return render_template('list_files.html', files=files)

@app.route('/uploads/<filename>')
def download_file(filename):
    return send_from_directory(app.config['UPLOAD_FOLDER'], filename)

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=2400 ,debug=True)
