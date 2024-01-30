# Use an official Python runtime as the base image
FROM python:3.9

WORKDIR /backend

# Copy the requirements file into the container
COPY requirements.txt .

# Install the Python dependencies
RUN pip install --no-cache-dir -r requirements.txt

# Copy the Flask app code into the container
COPY . .

# Expose the port on which the Flask app will run
EXPOSE 5000

# Set the environment variable for Flask
CMD ["python3", "-m" , "flask", "run", "--host=0.0.0.0"]