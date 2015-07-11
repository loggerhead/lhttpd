import os
from random import choice
from mimetypes import MimeTypes
import flask
from flask import Flask, redirect

image_dir = 'static/imgs/'
app = Flask(__name__)
images = os.listdir(os.path.join(app.static_folder, 'imgs'))
images = filter(lambda image: not image.startswith('.'), images)

@app.route('/' + image_dir + '<path>')
def _(**kwargs):
    return redirect("/", code=301)

@app.route('/random_image')
def random_image():
    filepath = os.path.join(image_dir, choice(images))
    data = open(filepath, 'rb').read()
    response = flask.make_response(data)
    response.headers['Content-Type'] = MimeTypes().guess_type(filepath)[0]
    return response

@app.route('/')
def index():
    return app.send_static_file('index.html')

if __name__ == '__main__':
    app.run(debug=True, host="0.0.0.0", port=8000)