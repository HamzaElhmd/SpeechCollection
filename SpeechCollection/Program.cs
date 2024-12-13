var builder = WebApplication.CreateBuilder(args);

// Add services to the container.
builder.Services.AddControllersWithViews();

var app = builder.Build();

// Configure the HTTP request pipeline.
if (!app.Environment.IsDevelopment())
{
    app.UseExceptionHandler("/Home/Error");
    // The default HSTS value is 30 days. You may want to change this for production scenarios, see https://aka.ms/aspnetcore-hsts.
    app.UseHsts();
}

app.UseHttpsRedirection();
app.UseStaticFiles();

app.UseRouting();

app.UseAuthorization();

app.MapControllerRoute(
    name: "default",
    pattern: "{controller=Home}/{action=Index}/{id?}");
app.MapControllerRoute(
    name: "default",
    pattern: "{controller=ConsentForm}/{action=Index}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=ConsentForm}/{action=Authenticate}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=ConsentForm}/{action=Verification}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=ConsentForm}/{action=Validate}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=SpeechDataCollection}/{action=Index}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=SpeechDataCollection}/{action=UploadFile}");
app.MapControllerRoute(
		name: "default",
		pattern: "{controller=SpeechDataCollection}/{action=Record}");
app.MapGet("/health", () => Results.Ok("Server is running"));

app.Run();
